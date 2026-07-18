#ifndef CONVEYOR_H
#define CONVEYOR_H
#include "machine.h"
#include "map.h"

#define CONVEYOR_MOVE_MS 500

typedef struct {
    int elapsedMs;
    int destinationX;
    int destinationY;
} ConveyorData;

int is_conveyor(Machine* machine);
void conveyor_update(Machine* machine, struct Map* map);
void* conveyor_get_data();
void conveyor_refresh(Machine* machine, struct Map* map);
void conveyor_refresh_near(struct Map* map, int x, int y);
int conveyor_try_dispatch_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs);
int conveyor_try_take_item(Machine* machine, struct Map* map, Slot* slot, int position, int* dispatchMs, int elapsedMs, DroppedItemFilter filter, void* context);

#endif
