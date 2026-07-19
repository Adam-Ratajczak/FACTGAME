#ifndef CONVEYOR_H
#define CONVEYOR_H
#include "machine.h"
#include "map.h"

int is_conveyor(Machine* machine);
int is_splitter(Machine* machine);
void conveyor_update(Machine* machine, struct Map* map);
void* conveyor_get_data();
void conveyor_refresh(Machine* machine, struct Map* map);
int conveyor_try_dispatch_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs);
int conveyor_try_take_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs, DroppedItemFilter filter, void* context);

void splitter_update(Machine* machine, struct Map* map);
void splitter_refresh(Machine* machine, struct Map* map);
void* splitter_get_data(TextureManager* texmgr);

Machine* conveyor_tunnel_create(TextureManager* texmgr, ItemRegistry* itemReg, int x1, int y1, int x2, int y2, int rotation);

#endif
