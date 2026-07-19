#ifndef SLOT_H
#define SLOT_H
#include "item.h"

#define HUD_SLOT_PURPOSE_NORMAL     0x0
#define HUD_SLOT_PURPOSE_CRAFT      0x1
#define HUD_SLOT_PURPOSE_ATTACK     0x2
#define HUD_SLOT_PURPOSE_MINE       0x3
#define HUD_SLOT_PURPOSE_BUILD      0x4

#define INVENTORY_COLS  6
#define INVENTORY_ROWS  3
#define CRAFTING_ROWS   3
#define SLOT_SIZE       20

typedef struct{
    int purpose;
    Item* item;
    Entity* sprite;
} Slot;

Slot* slot_create(TextureManager* texmgr, int purpose, int x, int y, Item* item);
void slot_destroy(Slot* slot);
void slot_set_item(Slot* slot, Item* item);
void slot_render(BITMAP* scr, Slot* slot);
int slot_get_texcoords(int purpose, int* left, int* top);

void slot_scale_up(Slot* slot);
void slot_scale_down(Slot* slot);

#endif