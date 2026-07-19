#include "crafters.h"
#include "conveyor.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FURNACE_CRAFT_MS 5000

int is_crafter_head(Machine* machine){
    return machine && machine->overlayId == OVERLAY_CRAFTER_HEAD;
}

int is_crafter_module(Machine* machine){
    return machine && machine->overlayId == OVERLAY_CRAFTER_MODULE;
}

typedef struct {
    int craftMs;
    int dispatchMs;
    int active;
} CrafterHeadData;

typedef struct {
    int inputMs;
} CrafterModuleData;

void* crafter_head_get_data(){
    CrafterHeadData* data = (CrafterHeadData*)malloc(sizeof(CrafterHeadData));
    if(!data)
        return NULL;

    data->active = 0;
    data->craftMs = 0;
    data->dispatchMs = 0;

    return data;
}

static int crafter_head_can_output(Slot* outputSlot, int outputItemId)
{
    if (!outputSlot)
        return 0;

    if (!outputSlot->item)
        return 1;

    return outputSlot->item->itemId == outputItemId &&
           outputSlot->item->amount < ITEM_STACK_SIZE;
}

static int crafter_head_add_output(Machine* machine, Slot* outputSlot, int outputItemId)
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

static void crafter_head_consume_input(Slot* inputSlot, int amount)
{
    inputSlot->item->amount -= amount;

    if (inputSlot->item->amount <= 0) {
        item_destroy(inputSlot->item);
        inputSlot->item = NULL;
    }
}

static int find_crafter_head_recipe(ItemRegistry* itemReg, Slot** inputSlots, int inputCount, ItemRecipe** recipe, int* outputItemId){
    if(!itemReg || !inputSlots || !recipe || !outputItemId){
        return 0;
    }

    for(int itemId = 0; itemId < ITEM_COUNT; itemId++){
        ItemRecipe* currRecipe = itemReg->info[itemId].recipe;
        if(!currRecipe || currRecipe->requiresCount != inputCount)
            continue;

        int matches = 0;
        for(int i = 0; i < inputCount; i++){
            ItemRecipeRequirement* requirement = &currRecipe->requires[i];
            for(int j = 0; j < inputCount; j++){
                Item* item = inputSlots[j]->item;
                if(!item){
                    continue;
                }
                if(requirement->itemId == item->itemId && item->amount >= requirement->amount){
                    matches++;
                }
            }
        }
        if(matches == inputCount){
            *recipe = currRecipe;
            *outputItemId = itemId;
            return 1;
        }
    }

    return 0;
}

static void crafter_consume_inputs(Slot** inputSlots, int inputCount, ItemRecipe* recipe){
    if(!inputSlots || !recipe || recipe->requiresCount != inputCount){
        return;
    }

    for(int i = 0; i < inputCount; i++){
        ItemRecipeRequirement* requirement = &recipe->requires[i];
        for(int j = 0; j < inputCount; j++){
            Item* item = inputSlots[j]->item;
            if(!item){
                continue;
            }
            if(requirement->itemId == item->itemId && item->amount >= requirement->amount){
                item->amount -= requirement->amount;

                if (item->amount <= 0) {
                    item_destroy(item);
                    inputSlots[j]->item = NULL;
                }
            }
        }
    }
}

void crafter_head_update(Machine* machine, struct Map* map){
    if(!machine || !machine->itemReg || !map || !machine->inventory || !machine->data)
        return;
    CrafterHeadData* data = (CrafterHeadData*)machine->data;

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

    Slot* outputSlot = machine->inventory->slots[0];

    int inputCount = 0;
    Slot** inputSlots = NULL;
    Machine* module = machine;
    while((module = machine_get_relative_to(module, map, MACHINE_POSITION_BOTTOM))){
        if(!is_crafter_module(module)){
            break;
        }

        if (!module->inventory || module->inventory->slotsCount < 1 || !module->inventory->slots[0])
            break;

        Slot** grown = realloc(inputSlots, (inputCount + 1) * sizeof(Slot*));
        if (!grown)
            break;

        inputSlots = grown;
        inputSlots[inputCount] = module->inventory->slots[0];
        inputCount++;
    }

    int outputItemId = -1;
    ItemRecipe* recipe = NULL;
    int canCraft = find_crafter_head_recipe(machine->itemReg, inputSlots, inputCount, &recipe, &outputItemId) && crafter_head_can_output(outputSlot, outputItemId);

    if (canCraft) {

        data->craftMs += elapsedMs;

        while (data->craftMs >= FURNACE_CRAFT_MS && canCraft) {
            data->craftMs -= FURNACE_CRAFT_MS;
            if (!crafter_head_add_output(machine, outputSlot, outputItemId))
                break;

            crafter_consume_inputs(inputSlots, inputCount, recipe);
            canCraft = find_crafter_head_recipe(machine->itemReg, inputSlots, inputCount, &recipe, &outputItemId) && crafter_head_can_output(outputSlot, outputItemId);
        }
    } else {
        data->craftMs = 0;
    }

    int active = canCraft;
    if (data->active != active) {
        data->active = active;
        machine_set_overlay_state(machine, map, active ? OVERLAY_CRAFTER_HEAD_ON : OVERLAY_CRAFTER_HEAD_OFF);
    }

    conveyor_try_dispatch_item(
        machine,
        map,
        outputSlot,
        MACHINE_POSITION_RIGHT,
        &data->dispatchMs,
        elapsedMs);

    free(inputSlots);
}

MachineInventory* crafter_head_create_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = malloc(sizeof(*machineInventory));
    if (!machineInventory)
        return NULL;

    strcpy(machineInventory->name, "Crafter Head");
    machineInventory->slots = malloc(sizeof(Slot*));
    machineInventory->slotsCount = 0;

    if (!machineInventory->slots) {
        free(machineInventory);
        return NULL;
    }

    machineInventory->slots[0] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_CRAFT,
        (SCREEN_W - SLOT_SIZE) / 2,
        (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2,
        NULL);

    if (!machineInventory->slots[0]) {
        free(machineInventory->slots);
        free(machineInventory);
        return NULL;
    }

    machineInventory->slotsCount = 1;
    return machineInventory;
}

void* crafter_module_get_data(){
    CrafterModuleData* data = malloc(sizeof(*data));
    if (!data)
        return NULL;

    data->inputMs = 0;
    return data;
}

void crafter_module_update(Machine* machine, struct Map* map){
    if (!machine || !map || !machine->inventory || !machine->data || machine->inventory->slotsCount < 1)
        return;

    CrafterModuleData* data = machine->data;
    clock_t now = clock();
    int elapsedMs = 0;

    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock)
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);

    machine->lastUpdateClock = now;

    crafter_module_refresh(machine, map);

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    conveyor_try_take_item(
        machine,
        map,
        machine->inventory->slots[0],
        MACHINE_POSITION_LEFT,
        &data->inputMs,
        elapsedMs,
        NULL,
        NULL);
}

MachineInventory* crafter_module_create_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = malloc(sizeof(*machineInventory));
    if (!machineInventory)
        return NULL;

    strcpy(machineInventory->name, "Crafter Module");
    machineInventory->slots = malloc(sizeof(Slot*));
    machineInventory->slotsCount = 0;

    if (!machineInventory->slots) {
        free(machineInventory);
        return NULL;
    }

    machineInventory->slots[0] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_NORMAL,
        (SCREEN_W - SLOT_SIZE) / 2,
        (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2,
        NULL);

    if (!machineInventory->slots[0]) {
        free(machineInventory->slots);
        free(machineInventory);
        return NULL;
    }

    machineInventory->slotsCount = 1;
    return machineInventory;
}

void crafter_module_refresh(Machine* machine, struct Map* map){
    if (!machine || !map || !is_crafter_module(machine))
        return;

    Machine* top = machine_get_relative_to(machine, map, MACHINE_POSITION_TOP);
    Machine* bottom = machine_get_relative_to(machine, map, MACHINE_POSITION_BOTTOM);
    int topIsHead = is_crafter_head(top);
    int topIsModule = is_crafter_module(top);
    int bottomIsModule = is_crafter_module(bottom);
    int overlay = OVERLAY_CRAFTER_MODULE_H;

    if (topIsHead && bottomIsModule)
        overlay = OVERLAY_CRAFTER_MODULE_HM;
    else if (topIsModule && bottomIsModule)
        overlay = OVERLAY_CRAFTER_MODULE_MM;
    else if (topIsModule)
        overlay = OVERLAY_CRAFTER_MODULE_M;
    else
        overlay = OVERLAY_CRAFTER_MODULE_H;

    machine_set_overlay_state(machine, map, overlay);
}
