#ifndef MACHINE_H
#define MACHINE_H
#include "entity.h"
#include "slot.h"

struct Map;
struct Machine;
typedef void (*MachineUpdateFunc)(struct Machine* machine, struct Map* map);

typedef struct {
    Slot** slots;
    int slotsCount;

    char name[32];
} MachineInventory;

typedef struct {
    int X;
    int Y;
    int rotation;
    int overlayId;
    MachineInventory* inventory;
    MachineUpdateFunc update;
} Machine;

Entity* get_preview_entity(TextureManager* texmgr, int overlayId);
int machine_get_tile_id(int overlayId);
Machine* machine_create(TextureManager* texmgr, int x, int y, int rotation, int overlayId);
void machine_destroy(Machine* machine);
void machine_update(Machine* machine, struct Map* map);

#endif
