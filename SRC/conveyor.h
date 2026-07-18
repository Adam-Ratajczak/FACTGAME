#ifndef CONVEYOR_H
#define CONVEYOR_H
#include "machine.h"
#include "map.h"

int is_conveyor(Machine* machine);
int is_splitter(Machine* machine);
void conveyor_update(Machine* machine, struct Map* map);
void* conveyor_get_data();
void conveyor_refresh(Machine* machine, struct Map* map);
void conveyor_refresh_near(struct Map* map, int x, int y);
int conveyor_try_dispatch_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs);
int conveyor_try_take_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs, DroppedItemFilter filter, void* context);

void splitter_update(Machine* machine, struct Map* map);
void splitter_refresh(Machine* machine, struct Map* map);
void* splitter_get_data(TextureManager* texmgr);

#endif
