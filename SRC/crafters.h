#ifndef CRAFTERS_H
#define CRAFTERS_H
#include "machine.h"

int is_crafter_head(Machine* machine);
int is_crafter_module(Machine* machine);

void* crafter_head_get_data();
void crafter_head_update(Machine* machine, struct Map* map);
MachineInventory* crafter_head_create_inventory(TextureManager* texmgr);

void* crafter_module_get_data();
void crafter_module_update(Machine* machine, struct Map* map);
MachineInventory* crafter_module_create_inventory(TextureManager* texmgr);
void crafter_module_refresh(Machine* machine, struct Map* map);

#endif
