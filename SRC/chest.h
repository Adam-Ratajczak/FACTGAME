#ifndef CHEST_H
#define CHEST_H
#include "machine.h"

void chest_update(Machine* machine, struct Map* map);
MachineInventory* chest_create_inventory(TextureManager* texmgr);
void* chest_get_data();

#endif
