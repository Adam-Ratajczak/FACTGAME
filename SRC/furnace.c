#include "furnace.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FURNACE_FUEL_MS 30000
#define FURNACE_SMELT_MS 5000

typedef struct {
    int fuelMs;
    int smeltMs;
    int active;
} FurnaceData;

void* furnace_get_data(){
    FurnaceData* data = (FurnaceData*)malloc(sizeof(FurnaceData));
    if(!data)
        return NULL;

    data->active = 0;
    data->fuelMs = 0;
    data->smeltMs = 0;

    return data;
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

void furnace_update(Machine* machine, struct Map* map)
{
    if(!machine || !machine->itemReg || !map || !machine->inventory || !machine->data || machine->inventory->slotsCount < 3)
        return;
    FurnaceData* data = (FurnaceData*)machine->data;

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

    if (canSmelt && data->fuelMs <= 0 && coalSlot && coalSlot->item && coalSlot->item->amount > 0) {
        coalSlot->item->amount--;
        data->fuelMs += FURNACE_FUEL_MS;

        if (coalSlot->item->amount <= 0) {
            item_destroy(coalSlot->item);
            coalSlot->item = NULL;
        }
    }

    if (data->fuelMs > 0 && canSmelt) {
        data->fuelMs -= elapsedMs;
        if (data->fuelMs < 0)
            data->fuelMs = 0;

        data->smeltMs += elapsedMs;

        while (data->smeltMs >= FURNACE_SMELT_MS && canSmelt) {
            data->smeltMs -= FURNACE_SMELT_MS;
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
        data->smeltMs = 0;
    }

    int active = data->fuelMs > 0 && canSmelt;

    if (data->active != active) {
        data->active = active;
        machine_set_overlay_state(machine, map, active ? OVERLAY_FURNACE_ON : OVERLAY_FURNACE_OFF);
    }
}

MachineInventory* furnace_create_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = (MachineInventory*)malloc(sizeof(MachineInventory));
    if(!machineInventory)
        return NULL;

    strcpy(machineInventory->name, "Furnace");
    machineInventory->slots = NULL;
    machineInventory->slotsCount = 0;

    int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
    int y = (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2;

    machineInventory->slots = malloc(sizeof(Slot*) * 3);
    if(!machineInventory->slots){
        free(machineInventory);
        return NULL;
    }

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
    for(int i = 0; i < machineInventory->slotsCount; ++i){
        if(machineInventory->slots[i])
            continue;

        for(int j = 0; j < i; ++j)
            slot_destroy(machineInventory->slots[j]);

        free(machineInventory->slots);
        free(machineInventory);
        return NULL;
    }

    return machineInventory;
}
