#include "chest.h"

#define CHEST_INVENTORY_ROWS 3

void chest_update(Machine* machine, struct Map* map){
    if(!machine || !map){
        return;
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

    int invHeight = (CHEST_INVENTORY_ROWS + INVENTORY_COLS) * SLOT_SIZE;
    int y = (SCREEN_H - invHeight) / 2;
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
