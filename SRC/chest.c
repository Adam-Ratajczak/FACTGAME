#include "chest.h"
#include "conveyor.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHEST_INVENTORY_ROWS 3

typedef struct {
    int inputMs;
    int dispatchMs;
} ChestData;

void* chest_get_data()
{
    ChestData* data = malloc(sizeof(*data));
    if (!data)
        return NULL;

    data->inputMs = 0;
    data->dispatchMs = 0;
    return data;
}

void chest_update(Machine* machine, struct Map* map){
    if(!machine || !map || !machine->inventory || !machine->data){
        return;
    }

    ChestData* data = machine->data;
    clock_t now = clock();
    int elapsedMs = 0;

    if (machine->lastUpdateClock != 0 && now > machine->lastUpdateClock)
        elapsedMs = (int)(((now - machine->lastUpdateClock) * 1000L) / CLOCKS_PER_SEC);

    machine->lastUpdateClock = now;

    if (elapsedMs <= 0)
        return;

    if (elapsedMs > 1000)
        elapsedMs = 1000;

    for (int i = 0; i < machine->inventory->slotsCount; ++i) {
        int addElapsed = i == 0 ? elapsedMs : 0;

        if (conveyor_try_take_item(
                machine,
                map,
                machine->inventory->slots[i],
                MACHINE_POSITION_LEFT,
                &data->inputMs,
                addElapsed,
                NULL,
                NULL))
            break;
    }

    for (int i = 0; i < machine->inventory->slotsCount; ++i) {
        Slot* slot = machine->inventory->slots[i];

        if (!slot || !slot->item)
            continue;

        conveyor_try_dispatch_item(
            machine,
            map,
            slot,
            MACHINE_POSITION_RIGHT,
            &data->dispatchMs,
            elapsedMs);
        break;
    }
}

MachineInventory* chest_create_inventory(TextureManager* texmgr){
    MachineInventory* machineInventory = (MachineInventory*)malloc(sizeof(MachineInventory));
    if(!machineInventory)
        return NULL;

    strcpy(machineInventory->name, "Chest");
    machineInventory->slots = NULL;
    machineInventory->slotsCount = 0;

    machineInventory->slots = malloc(sizeof(Slot*) * INVENTORY_COLS * CHEST_INVENTORY_ROWS);
    if(!machineInventory->slots){
        free(machineInventory);
        return NULL;
    }

    int y = (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS + 1) * SLOT_SIZE) / 2;
    for(int i = 0; i < CHEST_INVENTORY_ROWS; ++i){
        int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
        for(int j = 0; j < INVENTORY_COLS; ++j){
            machineInventory->slots[i * INVENTORY_COLS + j] = slot_create(texmgr, HUD_SLOT_PURPOSE_NORMAL, x, y, NULL);
            x += SLOT_SIZE;
        }
        y += SLOT_SIZE;
    }

    machineInventory->slotsCount = INVENTORY_COLS * CHEST_INVENTORY_ROWS;
    return machineInventory;
}
