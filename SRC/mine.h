#ifndef MINE_H
#define MINE_H
#include "machine.h"

void* mine_get_data();
void mine_update(Machine* machine, struct Map* map);
MachineInventory* mine_create_inventory(TextureManager* texmgr);

#endif