#ifndef FURNACE_H
#define FURNACE_H
#include "machine.h"

void furnace_update(Machine* machine, struct Map* map);
MachineInventory* furnace_create_inventory(TextureManager* texmgr);
void* furnace_get_data();

#endif