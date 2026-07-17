#include "machine.h"
#include "map.h"
#include <stdlib.h>
#include <string.h>

#define FURNACE_FUEL_MS 30000
#define FURNACE_SMELT_MS 5000

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

static void machine_set_overlay_state(Machine* machine, struct Map* map, int tileId)
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

static int find_furnace_recipe(ItemRegistry* itemReg, int inputItemId, int* outputItemId, int* inputAmount)
{
    if (!itemReg)
        return 0;

    for (int i = 0; i < ITEM_COUNT; ++i) {
        ItemInfo* info = &itemReg->info[i];

        if (!info->smelting || !info->recipe)
            continue;

        for (int j = 0; j < info->recipe->requiresCount; ++j) {
            ItemRecipeRequirement* requirement = &info->recipe->requires[j];

            if (requirement->itemId != inputItemId)
                continue;

            if (outputItemId)
                *outputItemId = i;
            if (inputAmount)
                *inputAmount = requirement->amount;

            return 1;
        }
    }

    return 0;
}

static int furnace_can_output(Slot* outputSlot, int outputItemId)
{
    if (!outputSlot)
        return 0;

    if (!outputSlot->item)
        return 1;

    return outputSlot->item->itemId == outputItemId &&
           outputSlot->item->amount < ITEM_STACK_SIZE;
}

static int furnace_add_output(Machine* machine, Slot* outputSlot, int outputItemId)
{
    if (!outputSlot->item) {
        Item* outputItem = item_create(machine->itemReg, machine->texmgr, outputItemId, 1);
        if (!outputItem)
            return 0;

        slot_set_item(outputSlot, outputItem);
        return 1;
    }

    outputSlot->item->amount++;
    return 1;
}

static void furnace_consume_input(Slot* inputSlot, int amount)
{
    inputSlot->item->amount -= amount;

    if (inputSlot->item->amount <= 0) {
        item_destroy(inputSlot->item);
        inputSlot->item = NULL;
    }
}

static void furnace_update(Machine* machine, struct Map* map)
{
    if(!machine || !machine->itemReg || !map || !machine->inventory || machine->inventory->slotsCount < 3)
        return;

    clock_t now = clock();
    int elapsedMs = 0;

    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock) {
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);
    }

    machine->lastUpdateClock = now;

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    Slot* normalA = machine->inventory->slots[0];
    Slot* normalB = machine->inventory->slots[1];
    Slot* outputSlot = machine->inventory->slots[2];
    Slot* coalSlot = NULL;
    Slot* inputSlot = NULL;

    if (normalA && normalA->item && normalA->item->itemId == ITEM_COAL)
        coalSlot = normalA;
    else if (normalB && normalB->item && normalB->item->itemId == ITEM_COAL)
        coalSlot = normalB;

    if (normalA && normalA != coalSlot && normalA->item)
        inputSlot = normalA;
    else if (normalB && normalB != coalSlot && normalB->item)
        inputSlot = normalB;

    int outputItemId = -1;
    int inputAmount = 0;
    int canSmelt = inputSlot &&
                   inputSlot->item &&
                   find_furnace_recipe(machine->itemReg, inputSlot->item->itemId, &outputItemId, &inputAmount) &&
                   inputSlot->item->amount >= inputAmount &&
                   furnace_can_output(outputSlot, outputItemId);

    if (canSmelt && machine->fuelMs <= 0 && coalSlot && coalSlot->item && coalSlot->item->amount > 0) {
        coalSlot->item->amount--;
        machine->fuelMs += FURNACE_FUEL_MS;

        if (coalSlot->item->amount <= 0) {
            item_destroy(coalSlot->item);
            coalSlot->item = NULL;
        }
    }

    if (machine->fuelMs > 0 && canSmelt) {
        machine->fuelMs -= elapsedMs;
        if (machine->fuelMs < 0)
            machine->fuelMs = 0;

        machine->smeltMs += elapsedMs;

        while (machine->smeltMs >= FURNACE_SMELT_MS && canSmelt) {
            machine->smeltMs -= FURNACE_SMELT_MS;
            if (!furnace_add_output(machine, outputSlot, outputItemId))
                break;

            furnace_consume_input(inputSlot, inputAmount);

            canSmelt = inputSlot &&
                       inputSlot->item &&
                       find_furnace_recipe(machine->itemReg, inputSlot->item->itemId, &outputItemId, &inputAmount) &&
                       inputSlot->item->amount >= inputAmount &&
                       furnace_can_output(outputSlot, outputItemId);
        }
    } else {
        machine->smeltMs = 0;
    }

    int active = machine->fuelMs > 0 && canSmelt;

    if (machine->active != active) {
        machine->active = active;
        machine_set_overlay_state(machine, map, active ? OVERLAY_FURNACE_ON : OVERLAY_FURNACE_OFF);
    }
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
