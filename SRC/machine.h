#ifndef MACHINE_H
#define MACHINE_H
#include "entity.h"
#include "slot.h"
#include <time.h>

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
    void* data;
    clock_t lastUpdateClock;
    TextureManager* texmgr;
    ItemRegistry* itemReg;
    MachineInventory* inventory;
    MachineUpdateFunc update;
    MachineUpdateFunc refresh;
} Machine;

Entity* get_preview_entity(TextureManager* texmgr, int overlayId);
int machine_get_tile_id(int overlayId);
Machine* machine_create(TextureManager* texmgr, ItemRegistry* itemReg, int x, int y, int rotation, int overlayId);
void machine_destroy(Machine* machine);
void machine_update(Machine* machine, struct Map* map);
void machine_refresh_near(struct Map* map, int x, int y);
void machine_set_overlay_state(Machine* machine, struct Map* map, int tileId);

#define MACHINE_POSITION_TOP 0
#define MACHINE_POSITION_RIGHT 1
#define MACHINE_POSITION_BOTTOM 2
#define MACHINE_POSITION_LEFT 3

Machine* machine_get_relative_to(Machine* machine, struct Map* map, int posId);

#endif
