#include "hud.h"

Slot* slot_create(TextureManager* texmgr, int purpose, int x, int y, Item* item){
    Slot* slot = (Slot*)malloc(sizeof(Slot));
    slot->purpose = purpose;

    int texLeft, texTop;
    if(!slot_get_texcoords(purpose, &texLeft, &texTop)){
        free(slot);
        return NULL;
    }

    slot->sprite = create_entity(x, y, 16, 16);
    add_sprite_to_entity(texmgr, slot->sprite, "ASSETS/TEXTURES/hud.pcx", texLeft, texTop);
    slot_set_item(slot, item);

    return slot;
}

void slot_destroy(Slot* slot){
    if(!slot){
        return;
    }

    destroy_entity(slot->sprite);
    item_destroy(slot->item);
    free(slot);
}

void slot_set_item(Slot* slot, Item* item){
    if(!slot){
        return;
    }

    if(item){
        slot->item = item;
        slot->item->inInventory = 1;
        slot->item->sprite->x = slot->sprite->x + 4;
        slot->item->sprite->y = slot->sprite->y + 4;
    }else{
        slot->item = NULL;
    }
}

void slot_render(BITMAP* scr, Slot* slot){
    if(!scr || !slot){
        return;
    }
    render_entity(scr, slot->sprite, NULL);
    if(slot->item){
        item_render(scr, slot->item, NULL);
    }
}

int slot_get_texcoords(int purpose, int* left, int* top){
    *top = 0;
    switch (purpose)
    {
    case HUD_SLOT_PURPOSE_NORMAL:
    case HUD_SLOT_PURPOSE_BUILD:
        *left = 48;
        return 1;
    case HUD_SLOT_PURPOSE_CRAFT:
        *left = 32;
        return 1;
    case HUD_SLOT_PURPOSE_ATTACK:
        *left = 0;
        return 1;
    case HUD_SLOT_PURPOSE_MINE:
        *left = 16;
        return 1;
    default:
        break;
    }

    return 0;
}

HUD* hud_create(ItemRegistry* itemReg, TextureManager* texmgr){
    HUD* hud = (HUD*)malloc(sizeof(HUD));
    int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
    int invHeight = (CRAFTING_ROWS + INVENTORY_COLS) * SLOT_SIZE;
    int y = (SCREEN_H - invHeight) / 2 + invHeight;

    hud->slots[ATTACK_SLOT] = slot_create(texmgr, HUD_SLOT_PURPOSE_ATTACK, x, y, item_create(itemReg, texmgr, ITEM_ATTACK_ORB, 1));
    x += SLOT_SIZE;
    hud->slots[MINING_SLOT] = slot_create(texmgr, HUD_SLOT_PURPOSE_MINE, x, y, item_create(itemReg, texmgr, ITEM_MINING_ORB, 1));
    x += SLOT_SIZE;
    for(int index = 2; index < INVENTORY_COLS; ++index){
        hud->slots[index] = slot_create(texmgr, HUD_SLOT_PURPOSE_BUILD, x, y, NULL);
        x += SLOT_SIZE;
    }
    slot_set_item(hud->slots[2], item_create(itemReg, texmgr, ITEM_FURNACE, 1));
    hud->selected = -1;

    hud_select_slot(hud, ATTACK_SLOT);
    return hud;
}
void hud_destroy(HUD* hud){
    for(int index = 0; index < INVENTORY_COLS; ++index){
        slot_destroy(hud->slots[index]);
    }

    free(hud);
}

void hud_select_slot(HUD* hud, int slotIndex){
    if(!hud || slotIndex < 0 || slotIndex >= INVENTORY_COLS){
        return;
    }

    if(hud->selected != -1){
        Slot* selSlot = hud->slots[hud->selected];
        selSlot->sprite->x += 2;
        selSlot->sprite->y += 2;
        selSlot->sprite->scale = 1.0;
    }

    Slot* selSlot = hud->slots[slotIndex];
    selSlot->sprite->x -= 2;
    selSlot->sprite->y -= 2;
    selSlot->sprite->scale = 1.25;

    hud->selected = slotIndex;
}

void hud_render(BITMAP* scr, HUD* hud){
    for(int index = 0; index < INVENTORY_COLS; ++index){
        slot_render(scr, hud->slots[index]);
    }
}

Inventory* inventory_create(ItemRegistry* itemReg, TextureManager* texmgr){
    Inventory* inventory = (Inventory*)malloc(sizeof(Inventory));
    inventory->hud = hud_create(itemReg, texmgr);
    inventory->shown = 0;
    inventory->hoveredInfo = NULL;

    int invHeight = (CRAFTING_ROWS + INVENTORY_COLS) * SLOT_SIZE;
    int y = (SCREEN_H - invHeight) / 2;
    for(int i = 0; i < CRAFTING_ROWS; ++i){
        int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
        for(int j = 0; j < INVENTORY_COLS; ++j){
            inventory->crafting[i * INVENTORY_COLS + j] = slot_create(texmgr, HUD_SLOT_PURPOSE_CRAFT, x, y, NULL);
            x += SLOT_SIZE;
        }
        y += SLOT_SIZE;
    }
    for(int i = 0; i < INVENTORY_ROWS; ++i){
        int x = (SCREEN_W - INVENTORY_COLS * SLOT_SIZE) / 2;
        for(int j = 0; j < INVENTORY_COLS; ++j){
            inventory->slots[i * INVENTORY_COLS + j] = slot_create(texmgr, HUD_SLOT_PURPOSE_NORMAL, x, y, NULL);
            x += SLOT_SIZE;
        }
        y += SLOT_SIZE;
    }

    return inventory;
}

void inventory_destroy(Inventory* inventory){
    if(!inventory){
        return;
    }
    hud_destroy(inventory->hud);

    for(int i = 0; i < CRAFTING_ROWS; ++i){
        for(int j = 0; j < INVENTORY_COLS; ++j){
            slot_destroy(inventory->crafting[i * INVENTORY_COLS + j]);
        }
    }
    for(int i = 0; i < INVENTORY_ROWS; ++i){
        for(int j = 0; j < INVENTORY_COLS; ++j){
            slot_destroy(inventory->slots[i * INVENTORY_COLS + j]);
        }
    }

    free(inventory);
}

int inventory_can_craft(ItemRegistry* itemReg, Inventory* inventory, int itemId){
    // TODO: Proper crafting
    return 1;
}

void inventory_show(ItemRegistry* itemReg, Inventory* inventory){
    if(!itemReg || !inventory){
        return;
    }

    inventory->shown = 1;
    if(inventory->hud->selected != -1){
        Slot* selSlot = inventory->hud->slots[inventory->hud->selected];
        selSlot->sprite->x += 2;
        selSlot->sprite->y += 2;
        selSlot->sprite->scale = 1.0;
    }

    // TODO: crafting table update
}
void inventory_hide(Inventory* inventory){
    inventory->shown = 0;
    if(inventory->hud->selected != -1){
        Slot* selSlot = inventory->hud->slots[inventory->hud->selected];
        selSlot->sprite->x -= 2;
        selSlot->sprite->y -= 2;
        selSlot->sprite->scale = 1.25;
    }
}

void inventory_render(BITMAP* scr, Inventory* inventory){
    if(!scr || !inventory){
        return;
    }

    if(inventory->shown){
        for(int i = 0; i < CRAFTING_ROWS; ++i){
            for(int j = 0; j < INVENTORY_COLS; ++j){
                slot_render(scr, inventory->crafting[i * INVENTORY_COLS + j]);
            }
        }
        for(int i = 0; i < INVENTORY_ROWS; ++i){
            for(int j = 0; j < INVENTORY_COLS; ++j){
                slot_render(scr, inventory->slots[i * INVENTORY_COLS + j]);
            }
        }
    }

    hud_render(scr, inventory->hud);

    if (inventory->hoveredInfo) {
        textout_ex(
            scr,
            font,
            inventory->hoveredInfo->name,
            4,
            4,
            makecol(255, 255, 255),
            makecol(0, 0, 0));
    }
}

Slot* inventory_get_selected_slot(Inventory* inventory){
    if(!inventory){
        return NULL;
    }

    int slotIndex = inventory->hud->selected;
    if(slotIndex < 0 || slotIndex >= INVENTORY_COLS){
        return NULL;
    }

    return inventory->hud->slots[slotIndex];
}

int inventory_pick_item(Inventory* inventory, Item* item)
{
    if (!inventory || !item)
        return 0;

    if (item_is_stackable(item))
    {
        for (int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS; ++i)
        {
            Slot* slot = inventory->slots[i];

            if (!slot || !slot->item)
                continue;

            if (slot->item->itemId != item->itemId)
                continue;

            if (slot->item->amount >= ITEM_STACK_SIZE)
                continue;

            int freeSpace = ITEM_STACK_SIZE - slot->item->amount;

            if (item->amount <= freeSpace)
            {
                slot->item->amount += item->amount;
                return 1;
            }

            slot->item->amount = ITEM_STACK_SIZE;
            item->amount -= freeSpace;
        }
    }

    for (int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS; ++i)
    {
        Slot* slot = inventory->slots[i];

        if (!slot || slot->item)
            continue;

        if (!item_is_stackable(item))
        {
            if (item->amount != 1)
                return 0;

            slot_set_item(slot, item);
            return 1;
        }

        slot_set_item(slot, item);
        return 1;
    }

    return 0;
}

Slot* inventory_get_slot_from_coords(Inventory* inventory, int x, int y){
    if(!inventory){
        return NULL;
    }

    for(int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS; i++){
        Slot* slot = inventory->slots[i];
        Entity* sprite = slot->sprite;
        if(sprite->x <= x && x <= sprite->x + sprite->w && sprite->y <= y && y <= sprite->y + sprite->h){
            return slot;
        }
    }

    for(int i = 0; i < INVENTORY_COLS * CRAFTING_ROWS; i++){
        Slot* slot = inventory->crafting[i];
        Entity* sprite = slot->sprite;
        if(sprite->x <= x && x <= sprite->x + sprite->w && sprite->y <= y && y <= sprite->y + sprite->h){
            return slot;
        }
    }

    for(int i = 0; i < INVENTORY_COLS; i++){
        Slot* slot = inventory->hud->slots[i];
        Entity* sprite = slot->sprite;
        if(sprite->x <= x && x <= sprite->x + sprite->w && sprite->y <= y && y <= sprite->y + sprite->h){
            return slot;
        }
    }

    return NULL;
}

void inventory_hover(ItemRegistry* itemReg, Inventory* inventory, int x, int y){
    if(!itemReg || !inventory){
        return;
    }
    Slot* slot = inventory_get_slot_from_coords(inventory, x, y);
    if(!slot || !slot->item){
        inventory->hoveredInfo = NULL;
        return;
    }

    inventory->hoveredInfo = &itemReg->info[slot->item->itemId];
}
