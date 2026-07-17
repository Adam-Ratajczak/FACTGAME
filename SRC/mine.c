#include "mine.h"

#define MINE_MINING_MS 3000

typedef struct {
    int miningMs;
    int active;
} MineData;

void* mine_get_data(){
    MineData* data = (MineData*)malloc(sizeof(MineData));
    data->miningMs = 0;
    data->active = 0;
    return data;
}

void mine_update(Machine* machine, struct Map* map)
{
    if (!machine || !machine->itemReg || !machine->data ||
        !machine->inventory || machine->inventory->slotsCount < 1 || !map)
        return;

    MineData* data = (MineData*)machine->data;

    clock_t now = clock();
    int elapsedMs = 0;

    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock)
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);

    machine->lastUpdateClock = now;

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    Slot* output = machine->inventory->slots[0];

    int minedItemId = -1;

    for (int z = 1; z >= 0 && minedItemId == -1; --z)
    {
        Tile* tile = get_tile(map, machine->X, machine->Y, z);
        if (!tile)
            continue;

        for (int itemId = 0; itemId < ITEM_COUNT; ++itemId)
        {
            if (machine->itemReg->info[itemId].aquiredFrom == -1)
                continue;

            if ((tile->TexID & 0xFFF0) == machine->itemReg->info[itemId].aquiredFrom)
            {
                minedItemId = itemId;
                break;
            }
        }
    }

    int active = 0;

    if (minedItemId != -1)
    {
        active = 1;

        if (output->item)
        {
            if (output->item->itemId != minedItemId)
            {
                item_destroy(output->item);
                output->item = NULL;
            }
            else if (output->item->amount >= ITEM_STACK_SIZE)
            {
                active = 0;
            }
        }

        if (active)
        {
            data->miningMs += elapsedMs;

            while (data->miningMs >= MINE_MINING_MS)
            {
                data->miningMs -= MINE_MINING_MS;

                if (!output->item)
                {
                    slot_set_item(output, item_create(machine->itemReg, machine->texmgr, minedItemId, 1));
                    output->item->amount = 1;
                }
                else
                {
                    if (output->item->amount >= ITEM_STACK_SIZE)
                    {
                        active = 0;
                        data->miningMs = 0;
                        break;
                    }

                    output->item->amount++;
                }
            }
        }
        else
        {
            data->miningMs = 0;
        }
    }
    else
    {
        data->miningMs = 0;
    }

    if (data->active != active)
    {
        data->active = active;
        machine_set_overlay_state(
            machine,
            map,
            OVERLAY_MINE);
    }
}

MachineInventory* mine_create_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = (MachineInventory*)malloc(sizeof(MachineInventory));
    if(!machineInventory)
        return NULL;

    strcpy(machineInventory->name, "Mine");
    machineInventory->slots = NULL;
    machineInventory->slotsCount = 0;

    machineInventory->slots = malloc(sizeof(Slot*));
    if(!machineInventory->slots){
        free(machineInventory);
        return NULL;
    }

    int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
    int y = (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2;
    machineInventory->slots[0] = slot_create(
        texmgr,
        HUD_SLOT_PURPOSE_CRAFT,
        x - SLOT_SIZE / 2,
        y + SLOT_SIZE / 2,
        NULL);

    machineInventory->slotsCount = 1;

    return machineInventory;
}
