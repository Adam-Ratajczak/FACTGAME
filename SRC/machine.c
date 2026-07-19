#include "machine.h"
#include "furnace.h"
#include "mine.h"
#include "chest.h"
#include "conveyor.h"
#include "crafters.h"
#include "map.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

int machine_get_tile_id(int overlayId)
{
    switch(overlayId){
        case OVERLAY_CONVEYOR_BELT:
            return OVERLAY_CONVEYOR_BELT_STRAIGHT;
        case OVERLAY_FURNACE:
            return OVERLAY_FURNACE_OFF;
        case OVERLAY_CRAFTER_HEAD:
            return OVERLAY_CRAFTER_HEAD_OFF;
        case OVERLAY_CRAFTER_MODULE:
            return OVERLAY_CRAFTER_MODULE_H;
        case OVERLAY_SPLITTER:
            return OVERLAY_SPLITTER_I;
        default:
            return overlayId;
    }
}

void machine_set_overlay_state(Machine* machine, struct Map* map, int tileId)
{
    Tile* tile = get_tile(map, machine->X, machine->Y, ZINDEX_OVERLAY);
    if (!tile || !tile->Entity)
        return;

    int texLeft = 0;
    int texTop = 0;

    if (!get_tile_def(tileId, &texLeft, &texTop))
        return;

    if (tile->TexID != tileId ||
        tile->Entity->sx != texLeft ||
        tile->Entity->sy != texTop) {
        tile->TexID = tileId;
        tile->Entity->sx = texLeft;
        tile->Entity->sy = texTop;
        map_invalidate_tile(map, machine->X, machine->Y);
    }
}

Machine* machine_create(TextureManager* texmgr, ItemRegistry* itemReg, int x, int y, int rotation, int overlayId)
{
    Machine* machine = calloc(1, sizeof(*machine));
    if (!machine)
        return NULL;

    machine->X = x;
    machine->Y = y;
    machine->rotation = rotation;
    machine->overlayId = overlayId;
    machine->texmgr = texmgr;
    machine->itemReg = itemReg;
    machine->inventory = NULL;
    machine->data = NULL;
    machine->update = NULL;
    machine->refresh = NULL;

    switch (overlayId) {
    case OVERLAY_CONVEYOR_BELT:
        machine->update = conveyor_update;
        machine->data = conveyor_get_data();
        machine->refresh = conveyor_refresh;
        break;
    case OVERLAY_FURNACE:
        machine->update = furnace_update;
        machine->inventory = furnace_create_inventory(texmgr);
        machine->data = furnace_get_data();
        break;
    case OVERLAY_MINE:
        machine->update = mine_update;
        machine->inventory = mine_create_inventory(texmgr);
        machine->data = mine_get_data();
        break;
    case OVERLAY_CHEST:
        machine->update = chest_update;
        machine->inventory = chest_create_inventory(texmgr);
        machine->data = chest_get_data();
        break;
    case OVERLAY_SPLITTER:
        machine->update = splitter_update;
        machine->data = splitter_get_data(texmgr);
        machine->refresh = splitter_refresh;
        break;
    case OVERLAY_CRAFTER_HEAD:
        machine->update = crafter_head_update;
        machine->inventory = crafter_head_create_inventory(texmgr);
        machine->data = crafter_head_get_data();
        break;
    case OVERLAY_CRAFTER_MODULE:
        machine->update = crafter_module_update;
        machine->inventory = crafter_module_create_inventory(texmgr);
        machine->data = crafter_module_get_data();
        machine->refresh = crafter_module_refresh;
        break;
    default:
        machine->update = NULL;
        break;
    }

    return machine;
}

void machine_destroy(Machine* machine)
{
    if (!machine)
        return;

    if(machine->inventory){
        for (int i = 0; i < machine->inventory->slotsCount; ++i)
            slot_destroy(machine->inventory->slots[i]);

        free(machine->inventory->slots);
        free(machine->inventory);
    }

    if(machine->data){
        free(machine->data);
    }

    free(machine);
}

void machine_update(Machine* machine, struct Map* map)
{
    if (machine && machine->update)
        machine->update(machine, map);
}

void machine_refresh(Machine* machine, struct Map* map){
    if (machine && machine->refresh)
        machine->refresh(machine, map);
}

void machine_refresh_near(Map* map, int x, int y)
{
    if (!map)
        return;

    Machine* machine = map_get_machine(map, x, y);
    machine_refresh(machine, map);

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if ((dx == 0 && dy == 0) || (dx != 0 && dy != 0))
                continue;
            machine = map_get_machine(map, x + dx, y + dy);
            machine_refresh(machine, map);
        }
    }
}

Entity* get_preview_entity(TextureManager* texmgr, int overlayId){
    int id = machine_get_tile_id(overlayId);

    int texLeft, texTop;
    if(!get_tile_def(id, &texLeft, &texTop)){
        return NULL;
    }

    Entity* entity = create_entity(0, 0, TILE_SIZE, TILE_SIZE);
    if (!entity)
        return NULL;

    add_sprite_to_entity(texmgr, entity, "ASSETS/TEXTURES/overlays.pcx", texLeft, texTop);

    return entity;
}

void machine_get_relative_position(Machine* machine, int posId, int* x, int* y)
{
    int dx = 0;
    int dy = 0;

    if (!machine) {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
    }

    switch ((posId + machine->rotation / 90) % 4)
    {
        case MACHINE_POSITION_TOP:
            dy = -1;
            break;

        case MACHINE_POSITION_RIGHT:
            dx = 1;
            break;

        case MACHINE_POSITION_BOTTOM:
            dy = 1;
            break;

        case MACHINE_POSITION_LEFT:
            dx = -1;
            break;
    }

    if (x) *x = machine->X + dx;
    if (y) *y = machine->Y + dy;
}

Machine* machine_get_relative_to(Machine* machine, struct Map* map, int posId)
{
    int x = 0;
    int y = 0;

    if (!machine || !map)
        return NULL;

    machine_get_relative_position(machine, posId, &x, &y);

    return map_get_machine(map, x, y);
}

void preview_direction(int rotation, int* dx, int* dy)
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
