#include "machine.h"
#include "furnace.h"
#include "mine.h"
#include "chest.h"
#include "conveyor.h"
#include "map.h"
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

    tile->TexID = tileId;
    tile->Entity->sx = texLeft;
    tile->Entity->sy = texTop;
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

    switch (overlayId) {
    case OVERLAY_CONVEYOR_BELT:
        machine->update = conveyor_update;
        machine->data = conveyor_get_data();
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

Machine* machine_get_relative_to(Machine* machine, struct Map* map, int posId)
{
    if (!machine || !map)
        return NULL;

    int dx = 0;
    int dy = 0;

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

    return map_get_machine(map, machine->X + dx, machine->Y + dy);
}
