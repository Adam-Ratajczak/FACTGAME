#ifndef MACHINE_H
#define MACHINE_H
#include "entity.h"
#include "item.h"

struct Map;
typedef struct Machine Machine;
typedef void (*MachineUpdateFunc)(Machine* machine, struct Map* map);

struct Machine {
    int X;
    int Y;
    int rotation;
    int overlayId;
    Item** inventory;
    int inventoryCount;
    MachineUpdateFunc update;
};

Entity* get_preview_entity(TextureManager* texmgr, int overlayId);
int machine_get_tile_id(int overlayId);
Machine* machine_create(int x, int y, int rotation, int overlayId);
void machine_destroy(Machine* machine);
void machine_update(Machine* machine, struct Map* map);

#endif
