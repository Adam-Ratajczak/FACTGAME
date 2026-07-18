#include "conveyor.h"
#include <stdlib.h>
#include <time.h>

int is_conveyor(Machine* machine)
{
    return machine && machine->overlayId == OVERLAY_CONVEYOR_BELT;
}

static int normalize_rotation(int rotation)
{
    rotation %= 360;
    if (rotation < 0)
        rotation += 360;

    return rotation;
}

static void conveyor_direction(int rotation, int* dx, int* dy)
{
    *dx = 0;
    *dy = 0;

    switch (normalize_rotation(rotation)) {
    case 90:
        *dy = 1;
        break;
    case 180:
        *dx = -1;
        break;
    case 270:
        *dy = -1;
        break;
    default:
        *dx = 1;
        break;
    }
}

static int conveyor_outputs_to(Machine* machine, int x, int y, int* fromDx, int* fromDy)
{
    int dx = 0;
    int dy = 0;

    if (!is_conveyor(machine))
        return 0;

    conveyor_direction(machine->rotation, &dx, &dy);
    if (machine->X + dx != x || machine->Y + dy != y)
        return 0;

    if (fromDx)
        *fromDx = machine->X - x;
    if (fromDy)
        *fromDy = machine->Y - y;

    return 1;
}

static int conveyor_turn_angle(int inDx, int inDy, int outDx, int outDy)
{
    if ((inDy < 0 && outDx > 0) || (inDx < 0 && outDy > 0))
        return 0;
    if ((inDx < 0 && outDy < 0) || (inDy > 0 && outDx > 0))
        return 90;
    if ((inDy > 0 && outDx < 0) || (inDx > 0 && outDy < 0))
        return 180;
    if ((inDx > 0 && outDy > 0) || (inDy < 0 && outDx < 0))
        return 270;

    return 0;
}

void* conveyor_get_data()
{
    ConveyorData* data = malloc(sizeof(*data));
    if (!data)
        return NULL;

    data->elapsedMs = 0;
    data->destinationX = 0;
    data->destinationY = 0;

    return data;
}

void conveyor_refresh(Machine* machine, struct Map* map)
{
    if (!is_conveyor(machine) || !map || !machine->data)
        return;

    int outDx = 0;
    int outDy = 0;
    int inDx = 0;
    int inDy = 0;
    int hasTurnInput = 0;

    conveyor_direction(machine->rotation, &outDx, &outDy);

    ConveyorData* data = machine->data;
    data->destinationX = machine->X + outDx;
    data->destinationY = machine->Y + outDy;

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int fromDx = 0;
            int fromDy = 0;
            Machine* neighbor = NULL;

            if ((dx == 0 && dy == 0) || (dx != 0 && dy != 0))
                continue;

            neighbor = map_get_machine(map, machine->X + dx, machine->Y + dy);
            if (!conveyor_outputs_to(neighbor, machine->X, machine->Y, &fromDx, &fromDy))
                continue;

            if (fromDx == -outDx && fromDy == -outDy)
                continue;

            inDx = fromDx;
            inDy = fromDy;
            hasTurnInput = 1;
        }
    }

    if (hasTurnInput) {
        int visualAngle = conveyor_turn_angle(inDx, inDy, outDx, outDy);
        machine_set_overlay_state(machine, map, OVERLAY_CONVEYOR_BELT_TURN);
        Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
        if (tile && tile->Entity)
            tile->Entity->angle = visualAngle;
    } else {
        machine_set_overlay_state(machine, map, OVERLAY_CONVEYOR_BELT_STRAIGHT);
        Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
        if (tile && tile->Entity)
            tile->Entity->angle = machine->rotation;
    }
}

void conveyor_refresh_near(Map* map, int x, int y)
{
    if (!map)
        return;

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx != 0 && dy != 0)
                continue;

            conveyor_refresh(map_get_machine(map, x + dx, y + dy), map);
        }
    }
}

void conveyor_update(Machine* machine, struct Map* map)
{
    if (!is_conveyor(machine) || !map || !machine->data)
        return;

    ConveyorData* data = machine->data;
    clock_t now = clock();
    int elapsedMs = 0;

    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock)
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);

    machine->lastUpdateClock = now;

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    conveyor_refresh(machine, map);

    data->elapsedMs += elapsedMs;
    if (data->elapsedMs > CONVEYOR_MOVE_MS)
        data->elapsedMs = CONVEYOR_MOVE_MS;

    if (data->elapsedMs < CONVEYOR_MOVE_MS)
        return;

    if (!is_conveyor(map_get_machine(map, data->destinationX, data->destinationY)))
        return;

    if (map_has_dropped_items(map, data->destinationX, data->destinationY))
        return;

    DroppedItems* items = map_release_dropped_items(map, machine->X, machine->Y);
    if (!items)
        return;

    if (map_place_dropped_items(map, items, data->destinationX, data->destinationY))
        data->elapsedMs -= CONVEYOR_MOVE_MS;
}

static int slot_can_accept(Slot* slot, int itemId)
{
    if (!slot)
        return 0;

    if (!slot->item)
        return 1;

    return slot->item->itemId == itemId && slot->item->amount < ITEM_STACK_SIZE;
}

static int add_item_to_slot(Slot* slot, Item* item)
{
    if (!slot || !item || !slot_can_accept(slot, item->itemId))
        return 0;

    if (!slot->item) {
        slot_set_item(slot, item);
        return 1;
    }

    slot->item->amount += item->amount;
    item_destroy(item);
    return 1;
}

int conveyor_try_dispatch_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs)
{
    Machine* conveyor = NULL;
    Item* moving = NULL;

    if (!machine || !map || !slot || !slot->item || !dispatchMs)
        return 0;

    *dispatchMs += elapsedMs;
    if (*dispatchMs > CONVEYOR_MOVE_MS)
        *dispatchMs = CONVEYOR_MOVE_MS;

    if (*dispatchMs < CONVEYOR_MOVE_MS)
        return 0;

    conveyor = machine_get_relative_to(machine, map, position);
    if (!is_conveyor(conveyor))
        return 0;

    if (map_has_dropped_items(map, conveyor->X, conveyor->Y))
        return 0;

    if (slot->item->amount > 1) {
        moving = item_create(machine->itemReg, machine->texmgr, slot->item->itemId, 1);
        if (!moving)
            return 0;

        slot->item->amount--;
    } else {
        moving = slot->item;
        slot->item = NULL;
    }

    if (!map_place_dropped_item(map, moving, conveyor->X, conveyor->Y)) {
        if (slot->item) {
            slot->item->amount++;
            item_destroy(moving);
        } else {
            slot_set_item(slot, moving);
        }
        return 0;
    }

    *dispatchMs -= CONVEYOR_MOVE_MS;
    return 1;
}

typedef struct {
    Slot* slot;
    DroppedItemFilter filter;
    void* context;
} TakeFilterContext;

static int can_take_to_slot(Item* item, void* context)
{
    TakeFilterContext* take = context;

    if (!item || !take || !slot_can_accept(take->slot, item->itemId))
        return 0;

    if (take->filter && !take->filter(item, take->context))
        return 0;

    return 1;
}

int conveyor_try_take_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs, DroppedItemFilter filter, void* context)
{
    Machine* conveyor = NULL;
    Item* item = NULL;
    TakeFilterContext take;

    if (!machine || !map || !slot || !dispatchMs)
        return 0;

    *dispatchMs += elapsedMs;
    if (*dispatchMs > CONVEYOR_MOVE_MS)
        *dispatchMs = CONVEYOR_MOVE_MS;

    if (*dispatchMs < CONVEYOR_MOVE_MS)
        return 0;

    conveyor = machine_get_relative_to(machine, map, position);
    if (!is_conveyor(conveyor))
        return 0;

    take.slot = slot;
    take.filter = filter;
    take.context = context;

    item = map_take_dropped_item(machine->itemReg, machine->texmgr, map, conveyor->X, conveyor->Y, can_take_to_slot, &take);
    if (!item)
        return 0;

    if (!add_item_to_slot(slot, item)) {
        map_place_dropped_item(map, item, conveyor->X, conveyor->Y);
        return 0;
    }

    *dispatchMs -= CONVEYOR_MOVE_MS;
    return 1;
}
