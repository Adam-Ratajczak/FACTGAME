#ifndef HUD_H
#define HUD_H
#include "machine.h"

#define ATTACK_SLOT     0
#define MINING_SLOT     1

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
    MachineInventory* machineInventory;
} Inventory;

Inventory* inventory_create(ItemRegistry* itemReg, TextureManager* texmgr);
void inventory_destroy(Inventory* inventory);
int inventory_can_craft(ItemRegistry* itemReg, Inventory* inventory, int itemId);
void inventory_show(ItemRegistry* itemReg, Inventory* inventory, MachineInventory* machineInventory);
void inventory_hide(Inventory* inventory);
void inventory_render(BITMAP* scr, Inventory* inventory);

Slot* inventory_get_selected_slot(Inventory* inventory);

int inventory_pick_item(Inventory* inventory, Item* item);
Slot* inventory_get_slot_from_coords(Inventory* inventory, int x, int y);
void inventory_hover(ItemRegistry* itemReg, Inventory* inventory, int x, int y);
void inventory_click(Inventory* inventory, int x, int y);

#endif
