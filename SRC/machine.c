#include "machine.h"
#include <stdlib.h>
#include <stdlib.h>

int machine_get_tile_id(int overlayId)
{
    switch(overlayId){
        case OVERLAY_CONVAYER_BELT:
            return OVERLAY_CONVAYER_BELT_STRAIGHT;
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

static void furnace_update(Machine* machine, struct Map* map)
{
    (void)machine;
    (void)map;
}

static MachineInventory* get_furnace_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = (MachineInventory*)malloc(sizeof(MachineInventory));
    strcpy(machineInventory->name, "Furnace");

    int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
    int y = (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2;

    machineInventory->slots = malloc(sizeof(Slot*) * 3);

    machineInventory->slots[0] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        x,
        y,
        NULL);

    machineInventory->slots[1] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        x,
        y + SLOT_SIZE,
        NULL);

    machineInventory->slots[2] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_CRAFT,
        x + SLOT_SIZE * 2,
        y + SLOT_SIZE / 2,
        NULL);

    machineInventory->slotsCount = 3;

    return machineInventory;
}

Machine* machine_create(TextureManager* texmgr, int x, int y, int rotation, int overlayId)
{
    Machine* machine = calloc(1, sizeof(*machine));
    if (!machine)
        return NULL;

    machine->X = x;
    machine->Y = y;
    machine->rotation = rotation;
    machine->overlayId = overlayId;
    machine->inventory = NULL;

    switch (overlayId) {
    case OVERLAY_FURNACE:
        machine->update = furnace_update;
        machine->inventory = get_furnace_inventory(texmgr);
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
