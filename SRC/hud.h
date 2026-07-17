#ifndef HUD_H
#define HUD_H
#include "item.h"

#define HUD_SLOT_PURPOSE_NORMAL     0x0
#define HUD_SLOT_PURPOSE_CRAFT      0x1
#define HUD_SLOT_PURPOSE_ATTACK     0x2
#define HUD_SLOT_PURPOSE_MINE       0x3
#define HUD_SLOT_PURPOSE_BUILD      0x4

#define ATTACK_SLOT     0
#define MINING_SLOT     1

#define INVENTORY_COLS  6
#define INVENTORY_ROWS  4
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

typedef struct {
    Slot* slots[INVENTORY_COLS];
    int selected;
} HUD;

HUD* hud_create(ItemRegistry* itemReg, TextureManager* texmgr);
void hud_destroy(HUD* hud);
void hud_select_slot(HUD* hud, int slotIndex);
void hud_render(BITMAP* scr, HUD* hud);

typedef struct {
    HUD* hud;
    Slot* slots[INVENTORY_COLS * INVENTORY_ROWS];
    Slot* crafting[INVENTORY_COLS * CRAFTING_ROWS];
    int shown;
    BITMAP* renderCache;
    unsigned long renderSignature;
    int renderCacheLeft;
    int renderCacheTop;
    int renderCacheWidth;
    int renderCacheHeight;
    Slot* selectedSlot;

    ItemInfo* hoveredInfo;
} Inventory;

Inventory* inventory_create(ItemRegistry* itemReg, TextureManager* texmgr);
void inventory_destroy(Inventory* inventory);
int inventory_can_craft(ItemRegistry* itemReg, Inventory* inventory, int itemId);
void inventory_show(ItemRegistry* itemReg, Inventory* inventory);
void inventory_hide(Inventory* inventory);
void inventory_render(BITMAP* scr, Inventory* inventory);

Slot* inventory_get_selected_slot(Inventory* inventory);

int inventory_pick_item(Inventory* inventory, Item* item);
Slot* inventory_get_slot_from_coords(Inventory* inventory, int x, int y);
void inventory_hover(ItemRegistry* itemReg, Inventory* inventory, int x, int y);
void inventory_click(Inventory* inventory, int x, int y);

#endif
