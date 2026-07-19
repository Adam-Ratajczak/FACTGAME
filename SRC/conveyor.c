#include "conveyor.h"
#include "log.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONVEYOR_MOVE_TICKS 30
#define CONVEYOR_MOVE_MS 500

typedef struct {
    int destinationX;
    int destinationY;
} ConveyorData;

int is_conveyor(Machine* machine)
{
    return machine && (machine->overlayId == OVERLAY_CONVEYOR_BELT || machine->overlayId == OVERLAY_CONVEYOR_TUNNEL);
}

int is_splitter(Machine* machine){
    return machine && machine->overlayId == OVERLAY_SPLITTER;
}

int conveyor_accepts_input_at(Machine* machine, int x, int y)
{
    if (!is_conveyor(machine))
        return 0;

    return machine->X == x && machine->Y == y;
}

static void conveyor_direction(int rotation, int* dx, int* dy)
{

    switch (normalize_rotation(rotation))
    {
    case 90:
        *dx = 1; *dy = 0;
        break;
    case 180:
        *dx = 0; *dy = 1;
        break;
    case 270:
        *dx = -1; *dy = 0;
        break;
    default:
        *dx = 0; *dy = -1;
        break;
    }
}

static int conveyor_outputs_to(Machine* machine, int x, int y, int* fromDx, int* fromDy)
{
    int dx = 0;
    int dy = 0;
    int outputDx = 0;
    int outputDy = 0;
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

    if (machine->overlayId == OVERLAY_CONVEYOR_TUNNEL &&
        machine->hasSecondaryPosition && data) {
        outputDx = data->destinationX - machine->secondaryX;
        outputDy = data->destinationY - machine->secondaryY;
    } else {
        outputDx = dx;
        outputDy = dy;
    }

    if (fromDx)
        *fromDx = -outputDx;
    if (fromDy)
        *fromDy = -outputDy;

    return 1;
}

static void conveyor_turn_transform(
    int inDx, int inDy,
    int outDx, int outDy,
    int* angle,
    int* scale)
{
    *scale = 1;

    if (inDx ==  1 && inDy ==  0 && outDx ==  0 && outDy ==  1) { *angle =   0; return; } // S->E
    if (inDx ==  0 && inDy ==  1 && outDx == -1 && outDy ==  0) { *angle =  90; return; } // W->S
    if (inDx == -1 && inDy ==  0 && outDx ==  0 && outDy == -1) { *angle = 180; return; } // N->W
    if (inDx ==  0 && inDy == -1 && outDx ==  1 && outDy ==  0) { *angle = 270; return; } // E->N

    *scale = -1;

    if (inDx ==  1 && inDy ==  0 && outDx ==  0 && outDy == -1) { *angle =   0; return; } // S->W
    if (inDx ==  0 && inDy ==  1 && outDx ==  1 && outDy ==  0) { *angle =  90; return; } // E->S
    if (inDx == -1 && inDy ==  0 && outDx ==  0 && outDy ==  1) { *angle = 180; return; } // N->E
    if (inDx ==  0 && inDy == -1 && outDx == -1 && outDy ==  0) { *angle = 270; return; } // W->N

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
    if (!machine || machine->overlayId != OVERLAY_CONVEYOR_BELT || !map || !machine->data)
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

    Machine* candidates[2];
    candidates[0] = machine_get_relative_to(machine, map, MACHINE_POSITION_RIGHT);
    candidates[1] = machine_get_relative_to(machine, map, MACHINE_POSITION_LEFT);

    for(int i = 0; i < 2; i++){
        Machine* candidate = candidates[i];

        if(is_conveyor(candidate)){
            int neighborDx = 0, neighborDy = 0;
            if (conveyor_outputs_to(candidate, machine->X, machine->Y, &neighborDx, &neighborDy)){
                if(outDx * neighborDx + outDy * neighborDy == 0){
                    inDx = -neighborDx;
                    inDy = -neighborDy;
                    hasTurnInput = 1;
                    break;
                }
            }
        }
    }

    if (hasTurnInput) {
        int angle;
        int scale;

        conveyor_turn_transform(inDx, inDy, outDx, outDy, &angle, &scale);

        machine_set_overlay_state(machine, map, scale > 0 ? OVERLAY_CONVEYOR_BELT_TURN_R : OVERLAY_CONVEYOR_BELT_TURN_L);

        Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
        if (tile && tile->Entity) {
            tile->Entity->angle = angle + 90;
        }
    } else {
        machine_set_overlay_state(machine, map, OVERLAY_CONVEYOR_BELT_STRAIGHT);
        Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
        if (tile && tile->Entity)
            tile->Entity->angle = machine->rotation;
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

    if (map->frame % CONVEYOR_MOVE_TICKS != 0)
        return;

    Machine* destination = map_get_machine(map, data->destinationX, data->destinationY);
    if (!conveyor_accepts_input_at(destination, data->destinationX, data->destinationY))
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

    if (!map_place_dropped_items(map, items, data->destinationX, data->destinationY)) {
        map_place_dropped_items(map, items, machine->X, machine->Y);
        return;
    }
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
    int targetX = 0;
    int targetY = 0;

    if (!machine || !map || !slot || !slot->item || !dispatchMs)
        return 0;

    *dispatchMs += elapsedMs;
    if (*dispatchMs > CONVEYOR_MOVE_MS)
        *dispatchMs = CONVEYOR_MOVE_MS;

    if (*dispatchMs < CONVEYOR_MOVE_MS)
        return 0;

    machine_get_relative_position(machine, position, &targetX, &targetY);
    conveyor = map_get_machine(map, targetX, targetY);
    if (!conveyor_accepts_input_at(conveyor, targetX, targetY))
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

typedef struct {
    int connected[4];
    int isInput[4];

    Slot* slots[4];
    int inputMs[4];
    int outputMs[4];
    int nextOutput;
} SplitterData;

void* splitter_get_data(TextureManager* texmgr)
{
    SplitterData* data = calloc(1, sizeof(*data));

    data->slots[0] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        0,
        0,
        NULL);

    data->slots[1] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        0,
        0,
        NULL);

    data->slots[2] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        0,
        0,
        NULL);

    data->slots[3] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        0,
        0,
        NULL);

    return data;
}

void splitter_refresh(Machine* machine, struct Map* map)
{
    if (!machine || !map || !machine->data || !is_splitter(machine))
        return;

    SplitterData* data = machine->data;
    memset(data->connected, 0, sizeof(data->connected));
    memset(data->isInput, 0, sizeof(data->isInput));

    int connectedCount = 0;

    for (int dir = MACHINE_POSITION_TOP; dir <= MACHINE_POSITION_LEFT; ++dir)
    {
        int neighborX = 0;
        int neighborY = 0;
        Machine* neighbor = machine_get_relative_to(machine, map, dir);
        if (!is_conveyor(neighbor))
            continue;

        machine_get_relative_position(machine, dir, &neighborX, &neighborY);
        if (!conveyor_outputs_to(neighbor, machine->X, machine->Y, NULL, NULL) &&
            !conveyor_accepts_input_at(neighbor, neighborX, neighborY))
            continue;

        if (neighbor->overlayId == OVERLAY_CONVEYOR_BELT)
            conveyor_refresh(neighbor, map);

        data->connected[dir] = 1;
        connectedCount++;
    }

    int overlay = OVERLAY_SPLITTER_I;
    int angle = 0;

    if (connectedCount == 4)
    {
        overlay = OVERLAY_SPLITTER_ALL;
    }
    else if (connectedCount == 3)
    {
        overlay = OVERLAY_SPLITTER_T;

        if (!data->connected[MACHINE_POSITION_LEFT])
            angle = 0;
        else if (!data->connected[MACHINE_POSITION_TOP])
            angle = 90;
        else if (!data->connected[MACHINE_POSITION_RIGHT])
            angle = 180;
        else
            angle = 270;
    }
    else
    {
        overlay = OVERLAY_SPLITTER_I;

        if ((data->connected[MACHINE_POSITION_LEFT] ||
             data->connected[MACHINE_POSITION_RIGHT]) &&
            !data->connected[MACHINE_POSITION_TOP] &&
            !data->connected[MACHINE_POSITION_BOTTOM])
            angle = 90;
        else
            angle = 0;
    }

    angle = normalize_rotation(angle + machine->rotation);

    machine_set_overlay_state(machine, map, overlay);

    Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
    if (tile && tile->Entity && tile->Entity->angle != angle) {
        tile->Entity->angle = angle;
        map_invalidate_tile(map, machine->X, machine->Y);
    }
}

void splitter_update(Machine* machine, struct Map* map)
{
    if (!machine || !map || !machine->data)
        return;

    SplitterData* data = machine->data;

    clock_t now = clock();
    int elapsedMs = 0;
    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock)
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);

    machine->lastUpdateClock = now;

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    splitter_refresh(machine, map);

    int outputs[4];
    int inputs[4];
    int outputCount = 0;
    int inputCount = 0;

    for (int pos = MACHINE_POSITION_TOP;
        pos <= MACHINE_POSITION_LEFT;
        ++pos)
    {
        Machine* neighbor;

        if (!data->connected[pos])
            continue;

        neighbor = machine_get_relative_to(machine, map, pos);
        data->isInput[pos] = conveyor_outputs_to(
            neighbor,
            machine->X,
            machine->Y,
            NULL,
            NULL);

        if (data->isInput[pos])
            inputs[inputCount++] = pos;
        else
            outputs[outputCount++] = pos;
    }

    log_debug("splitter (%d,%d) inputs=%d outputs=%d", machine->X, machine->Y, inputCount, outputCount);

    if (!outputCount)
        return;

    for (int i = 0; i < inputCount; ++i)
    {
        int pos = inputs[i];

        Slot* slot = data->slots[pos];
        if (!slot || slot->item)
            continue;

        conveyor_try_take_item(
            machine,
            map,
            slot,
            pos,
            &data->inputMs[pos],
            elapsedMs,
            NULL,
            NULL);
    }

    for (int pos = MACHINE_POSITION_TOP; pos <= MACHINE_POSITION_LEFT; ++pos)
    {
        Slot* slot = data->slots[pos];
        if (!slot || !slot->item)
            continue;

        for (int i = 0; i < outputCount; ++i)
        {
            int out = outputs[(data->nextOutput + i) % outputCount];

            if (conveyor_try_dispatch_item(
                machine,
                map,
                slot,
                out,
                &data->outputMs[out],
                elapsedMs))
            {
                data->nextOutput = (data->nextOutput + i + 1) % outputCount;
                break;
            }
        }
    }
}

Machine* conveyor_tunnel_create(TextureManager* texmgr, ItemRegistry* itemReg, int x1, int y1, int x2, int y2, int rotation){
    Machine* machine = calloc(1, sizeof(*machine));
    if (!machine)
        return NULL;

    machine->X = x1;
    machine->Y = y1;
    machine->secondaryX = x2;
    machine->secondaryY = y2;
    machine->hasSecondaryPosition = 1;
    machine->rotation = rotation;
    machine->overlayId = OVERLAY_CONVEYOR_TUNNEL;
    machine->texmgr = texmgr;
    machine->itemReg = itemReg;
    machine->inventory = NULL;
    machine->update = conveyor_update;
    machine->refresh = NULL;

    ConveyorData* data = malloc(sizeof(*data));
    if (!data){
        free(machine);
        return NULL;
    }

    int dx = 0, dy = 0;
    preview_direction(rotation, &dx, &dy);
    data->destinationX = x2 + dx;
    data->destinationY = y2 + dy;
    machine->data = data;

    return machine;
}
