#include "conveyor.h"
#include <stdlib.h>
#include <time.h>

#define CONVEYOR_MOVE_TICKS 30
#define CONVEYOR_MOVE_MS 500

typedef struct {
    int destinationX;
    int destinationY;
} ConveyorData;

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

static int machine_index(Map* map, Machine* machine)
{
    if (!map || !machine)
        return -1;

    for (int i = 0; i < map->machineCount; ++i) {
        if (map->machines[i] == machine)
            return i;
    }

    return -1;
}

static int conveyor_outputs_to(Machine* machine, int x, int y, int* fromDx, int* fromDy)
{
    int dx = 0;
    int dy = 0;
    ConveyorData* data = NULL;

    if (!is_conveyor(machine))
        return 0;

    data = machine->data;
    if (data) {
        dx = data->destinationX - machine->X;
        dy = data->destinationY - machine->Y;
    } else {
        conveyor_direction(machine->rotation, &dx, &dy);
    }

    if (machine->X + dx != x || machine->Y + dy != y)
        return 0;

    if (fromDx)
        *fromDx = machine->X - x;
    if (fromDy)
        *fromDy = machine->Y - y;

    return 1;
}

static int conveyor_select_output(Machine* machine, Map* map, int* outDx, int* outDy)
{
    int dx = 0;
    int dy = 0;
    int selfIndex = machine_index(map, machine);

    for (int i = selfIndex + 1; i < map->machineCount; ++i) {
        Machine* candidate = map->machines[i];

        if (!is_conveyor(candidate))
            continue;

        dx = candidate->X - machine->X;
        dy = candidate->Y - machine->Y;

        if ((dx == 0 && (dy == -1 || dy == 1)) ||
            (dy == 0 && (dx == -1 || dx == 1))) {
            *outDx = dx;
            *outDy = dy;
            return 1;
        }
    }

    conveyor_direction(machine->rotation, &dx, &dy);
    if (is_conveyor(map_get_machine(map, machine->X + dx, machine->Y + dy))) {
        *outDx = dx;
        *outDy = dy;
        return 1;
    }

    return 0;
}

static void conveyor_turn_transform(
    int inDx, int inDy,
    int outDx, int outDy,
    int* angle,
    int* scale)
{
    *scale = 1;

    // Right turns (base sprite and its rotations)
    if (inDx == 0 && inDy == 1 && outDx == 1 && outDy == 0) {        // S -> E
        *angle = 0;
        return;
    }
    if (inDx == -1 && inDy == 0 && outDx == 0 && outDy == 1) {       // W -> S
        *angle = 90;
        return;
    }
    if (inDx == 0 && inDy == -1 && outDx == -1 && outDy == 0) {      // N -> W
        *angle = 180;
        return;
    }
    if (inDx == 1 && inDy == 0 && outDx == 0 && outDy == -1) {       // E -> N
        *angle = 270;
        return;
    }

    // Left turns (mirror the sprite)
    *scale = -1;

    if (inDx == 0 && inDy == 1 && outDx == -1 && outDy == 0) {       // S -> W
        *angle = 0;
        return;
    }
    if (inDx == -1 && inDy == 0 && outDx == 0 && outDy == -1) {      // W -> N
        *angle = 90;
        return;
    }
    if (inDx == 0 && inDy == -1 && outDx == 1 && outDy == 0) {       // N -> E
        *angle = 180;
        return;
    }
    if (inDx == 1 && inDy == 0 && outDx == 0 && outDy == 1) {        // E -> S
        *angle = 270;
        return;
    }

    *angle = 0;
    *scale = 1;
}

void* conveyor_get_data()
{
    ConveyorData* data = malloc(sizeof(*data));
    if (!data)
        return NULL;

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

    if (!conveyor_select_output(machine, map, &outDx, &outDy)) {
        conveyor_direction(machine->rotation, &outDx, &outDy);
    }

    ConveyorData* data = machine->data;
    data->destinationX = machine->X + outDx;
    data->destinationY = machine->Y + outDy;

    Machine* previous = NULL;
    int prevOutDx = 0;
    int prevOutDy = 0;

    for (int dy = -1; dy <= 1 && !previous; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if ((dx == 0 && dy == 0) || (dx != 0 && dy != 0))
                continue;

            Machine* neighbor = map_get_machine(map, machine->X + dx, machine->Y + dy);
            if (!is_conveyor(neighbor))
                continue;

            int dummyDx, dummyDy;
            if (!conveyor_outputs_to(neighbor, machine->X, machine->Y, &dummyDx, &dummyDy))
                continue;

            previous = neighbor;

            if (!conveyor_select_output(previous, map, &prevOutDx, &prevOutDy))
                conveyor_direction(previous->rotation, &prevOutDx, &prevOutDy);
            break;
        }
    }

    int hasOutput = is_conveyor(map_get_machine(map, machine->X + outDx, machine->Y + outDy));
    if (previous && hasOutput && (prevOutDx != outDx || prevOutDy != outDy))
    {
        inDx = -prevOutDx;
        inDy = -prevOutDy;
        hasTurnInput = 1;
    }

    if (hasTurnInput) {
        int angle;
        int scale;

        conveyor_turn_transform(inDx, inDy, outDx, outDy, &angle, &scale);

        machine_set_overlay_state(machine, map, scale > 0 ? OVERLAY_CONVEYOR_BELT_TURN_R : OVERLAY_CONVEYOR_BELT_TURN_L);

        Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
        if (tile && tile->Entity) {
            tile->Entity->angle = angle;
        }
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

    if (map->frame % CONVEYOR_MOVE_TICKS != 0)
        return;

    if (!is_conveyor(map_get_machine(map, data->destinationX, data->destinationY)))
        return;

    if (map_has_dropped_items(map, data->destinationX, data->destinationY))
        return;

    DroppedItems* items = map_release_dropped_items(map, machine->X, machine->Y);
    if (!items)
        return;

    if (items->lastMoveFrame == map->frame){
        map_place_dropped_items(map, items, machine->X, machine->Y);
        return;
    }

    if (!map_place_dropped_items(map, items, data->destinationX, data->destinationY))
        return;
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
