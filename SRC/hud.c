#include "hud.h"

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
        slot_scale_down(hud->slots[hud->selected]);
    }

    slot_scale_up(hud->slots[slotIndex]);
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
    inventory->renderCache = NULL;
    inventory->renderSignature = 0;
    inventory->renderCacheLeft = 0;
    inventory->renderCacheTop = 0;
    inventory->renderCacheWidth = 0;
    inventory->renderCacheHeight = 0;
    inventory->hoveredInfo = NULL;
    inventory->selectedSlot = NULL;
    inventory->texmgr = texmgr;
    inventory->machineInventory = NULL;
    for(int i = 0; i < INVENTORY_COLS * CRAFTING_ROWS; ++i){
        inventory->craftingItemIds[i] = -1;
    }

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

    if (inventory->renderCache) {
        destroy_bitmap(inventory->renderCache);
    }

    free(inventory);
}

static int inventory_count_item(Inventory* inventory, int itemId)
{
    int count = 0;

    if(!inventory)
        return 0;

    for(int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS; ++i){
        Slot* slot = inventory->slots[i];

        if(slot && slot->item && slot->item->itemId == itemId)
            count += slot->item->amount;
    }

    return count;
}

int inventory_can_craft(ItemRegistry* itemReg, Inventory* inventory, int itemId){
    if(!itemReg || !inventory || itemId < 0 || itemId >= ITEM_COUNT)
        return 0;

    ItemInfo* info = &itemReg->info[itemId];
    if(!info->recipe || info->smelting)
        return 0;

    for(int i = 0; i < info->recipe->requiresCount; ++i){
        ItemRecipeRequirement* requirement = &info->recipe->requires[i];

        if(inventory_count_item(inventory, requirement->itemId) < requirement->amount)
            return 0;
    }

    return 1;
}

static void inventory_clear_crafting(Inventory* inventory)
{
    if(!inventory)
        return;

    for(int i = 0; i < INVENTORY_COLS * CRAFTING_ROWS; ++i){
        if(inventory->crafting[i]->item){
            item_destroy(inventory->crafting[i]->item);
            slot_set_item(inventory->crafting[i], NULL);
        }
        inventory->craftingItemIds[i] = -1;
    }
}

static void inventory_update_crafting(ItemRegistry* itemReg, Inventory* inventory)
{
    int slotIndex = 0;

    if(!itemReg || !inventory || inventory->machineInventory)
        return;

    inventory_clear_crafting(inventory);

    for(int itemId = 0; itemId < ITEM_COUNT && slotIndex < INVENTORY_COLS * CRAFTING_ROWS; ++itemId){
        if(!inventory_can_craft(itemReg, inventory, itemId))
            continue;

        Item* preview = item_create(itemReg, inventory->texmgr, itemId, 1);
        if(!preview)
            continue;

        slot_set_item(inventory->crafting[slotIndex], preview);
        inventory->craftingItemIds[slotIndex] = itemId;
        slotIndex++;
    }
}

static int inventory_consume_item(Inventory* inventory, int itemId, int amount)
{
    if(!inventory || amount <= 0)
        return 0;

    for(int i = 0; i < INVENTORY_COLS * INVENTORY_ROWS && amount > 0; ++i){
        Slot* slot = inventory->slots[i];

        if(!slot || !slot->item || slot->item->itemId != itemId)
            continue;

        if(slot->item->amount > amount){
            slot->item->amount -= amount;
            amount = 0;
        }else{
            amount -= slot->item->amount;
            item_destroy(slot->item);
            slot_set_item(slot, NULL);
        }
    }

    return amount == 0;
}

static int inventory_craft_item(ItemRegistry* itemReg, Inventory* inventory, int itemId)
{
    if(!inventory_can_craft(itemReg, inventory, itemId))
        return 0;

    Item* crafted = item_create(itemReg, inventory->texmgr, itemId, 1);
    if(!crafted)
        return 0;

    if(!inventory_pick_item(inventory, crafted)){
        item_destroy(crafted);
        return 0;
    }

    ItemRecipe* recipe = itemReg->info[itemId].recipe;
    for(int i = 0; i < recipe->requiresCount; ++i){
        ItemRecipeRequirement* requirement = &recipe->requires[i];
        inventory_consume_item(inventory, requirement->itemId, requirement->amount);
    }

    return 1;
}

void inventory_show(ItemRegistry* itemReg, Inventory* inventory, MachineInventory* machineInventory){
    if(!itemReg || !inventory){
        return;
    }

    inventory->shown = 1;
    inventory->renderSignature = 0;
    inventory->machineInventory = machineInventory;
    inventory_clear_crafting(inventory);
    if(inventory->hud->selected != -1){
        slot_scale_down(inventory->hud->slots[inventory->hud->selected]);
    }

    if(!machineInventory)
        inventory_update_crafting(itemReg, inventory);
}
void inventory_hide(Inventory* inventory){
    inventory->shown = 0;
    inventory->renderSignature = 0;
    inventory->machineInventory = NULL;
    inventory->hoveredInfo = NULL;
    inventory_clear_crafting(inventory);
    if(inventory->hud->selected != -1){
        slot_scale_up(inventory->hud->slots[inventory->hud->selected]);
    }
    if(inventory->selectedSlot){
        slot_scale_down(inventory->selectedSlot);
    }
}

static unsigned long slot_render_signature(Slot* slot)
{
    unsigned long sig = 2166136261u;

    if (!slot)
        return sig;

    sig = (sig ^ (unsigned long)slot->purpose) * 16777619u;
    sig = (sig ^ (unsigned long)slot->sprite->x) * 16777619u;
    sig = (sig ^ (unsigned long)slot->sprite->y) * 16777619u;
    sig = (sig ^ (unsigned long)(slot->sprite->scale * 100.0f)) * 16777619u;

    if (slot->item) {
        sig = (sig ^ (unsigned long)(slot->item->itemId + 1)) * 16777619u;
        sig = (sig ^ (unsigned long)slot->item->amount) * 16777619u;
        sig = (sig ^ (unsigned long)slot->item->durability) * 16777619u;
    }

    return sig;
}

static unsigned long inventory_render_signature(Inventory* inventory)
{
    unsigned long sig = 2166136261u;

    sig = (sig ^ (unsigned long)inventory->shown) * 16777619u;
    sig = (sig ^ (unsigned long)(inventory->hud ? inventory->hud->selected + 1 : 0)) * 16777619u;

    if (inventory->shown) {
        if(inventory->machineInventory){
            for(int i = 0; i < inventory->machineInventory->slotsCount; ++i)
                sig = (sig ^ slot_render_signature(inventory->machineInventory->slots[i])) * 16777619u;
        }else{
            for(int i = 0; i < CRAFTING_ROWS * INVENTORY_COLS; ++i) {
                sig = (sig ^ slot_render_signature(inventory->crafting[i])) * 16777619u;
            }
        }

        for(int i = 0; i < INVENTORY_ROWS * INVENTORY_COLS; ++i)
            sig = (sig ^ slot_render_signature(inventory->slots[i])) * 16777619u;
    }

    for(int i = 0; i < INVENTORY_COLS; ++i)
        sig = (sig ^ slot_render_signature(inventory->hud->slots[i])) * 16777619u;

    return sig;
}

static void expand_slot_bounds(Slot* slot, int* left, int* top, int* right, int* bottom)
{
    if (!slot || !slot->sprite)
        return;

    int slotRight = slot->sprite->x + (int)(slot->sprite->w * slot->sprite->scale) + 1;
    int slotBottom = slot->sprite->y + (int)(slot->sprite->h * slot->sprite->scale) + 1;

    if (slot->sprite->x < *left) *left = slot->sprite->x;
    if (slot->sprite->y < *top) *top = slot->sprite->y;
    if (slotRight > *right) *right = slotRight;
    if (slotBottom > *bottom) *bottom = slotBottom;

    if (slot->item && slot->item->sprite) {
        int itemRight = slot->item->sprite->x + slot->item->sprite->w + 10;
        int itemBottom = slot->item->sprite->y + slot->item->sprite->h + 2;

        if (slot->item->sprite->x < *left) *left = slot->item->sprite->x;
        if (slot->item->sprite->y < *top) *top = slot->item->sprite->y;
        if (itemRight > *right) *right = itemRight;
        if (itemBottom > *bottom) *bottom = itemBottom;
    }
}

static void inventory_get_render_bounds(Inventory* inventory, int* left, int* top, int* width, int* height)
{
    int right = 0;
    int bottom = 0;

    *left = SCREEN_W;
    *top = SCREEN_H;

    if (inventory->shown) {
        if(inventory->machineInventory){
            for(int i = 0; i < inventory->machineInventory->slotsCount; ++i) {
                expand_slot_bounds(inventory->machineInventory->slots[i], left, top, &right, &bottom);
            }
        }else{
            for(int i = 0; i < CRAFTING_ROWS * INVENTORY_COLS; ++i) {
                expand_slot_bounds(inventory->crafting[i], left, top, &right, &bottom);
            }
        }

        for(int i = 0; i < INVENTORY_ROWS * INVENTORY_COLS; ++i)
            expand_slot_bounds(inventory->slots[i], left, top, &right, &bottom);
    }

    for(int i = 0; i < INVENTORY_COLS; ++i)
        expand_slot_bounds(inventory->hud->slots[i], left, top, &right, &bottom);

    *left -= 4;
    *top -= 4;
    right += 4;
    bottom += 4;

    if (*left < 0) *left = 0;
    if (*top < 0) *top = 0;
    if (right > SCREEN_W) right = SCREEN_W;
    if (bottom > SCREEN_H) bottom = SCREEN_H;

    *width = right - *left;
    *height = bottom - *top;

    if (*width <= 0) *width = 1;
    if (*height <= 0) *height = 1;
}

static void slot_render_shifted(BITMAP* scr, Slot* slot, int originX, int originY)
{
    if (!slot)
        return;

    int slotX = slot->sprite->x;
    int slotY = slot->sprite->y;
    int itemX = 0;
    int itemY = 0;

    slot->sprite->x -= originX;
    slot->sprite->y -= originY;

    if (slot->item) {
        itemX = slot->item->sprite->x;
        itemY = slot->item->sprite->y;
        slot->item->sprite->x -= originX;
        slot->item->sprite->y -= originY;
    }

    slot_render(scr, slot);

    slot->sprite->x = slotX;
    slot->sprite->y = slotY;

    if (slot->item) {
        slot->item->sprite->x = itemX;
        slot->item->sprite->y = itemY;
    }
}

static int inventory_rebuild_render_cache(Inventory* inventory, unsigned long signature)
{
    inventory_get_render_bounds(
        inventory,
        &inventory->renderCacheLeft,
        &inventory->renderCacheTop,
        &inventory->renderCacheWidth,
        &inventory->renderCacheHeight);

    if (inventory->renderCache &&
        (inventory->renderCache->w != inventory->renderCacheWidth ||
         inventory->renderCache->h != inventory->renderCacheHeight)) {
        destroy_bitmap(inventory->renderCache);
        inventory->renderCache = NULL;
    }

    if (!inventory->renderCache) {
        inventory->renderCache = create_bitmap(inventory->renderCacheWidth, inventory->renderCacheHeight);
        if (!inventory->renderCache)
            return 0;
    }

    clear_to_color(inventory->renderCache, inventory->renderCache->vtable->mask_color);

    if(inventory->shown){
        if(inventory->machineInventory){
            for(int i = 0; i < inventory->machineInventory->slotsCount; ++i) {
                slot_render_shifted(inventory->renderCache, inventory->machineInventory->slots[i], inventory->renderCacheLeft, inventory->renderCacheTop);
            }
        }else{
            for(int i = 0; i < CRAFTING_ROWS; ++i){
                for(int j = 0; j < INVENTORY_COLS; ++j){
                    slot_render_shifted(inventory->renderCache, inventory->crafting[i * INVENTORY_COLS + j], inventory->renderCacheLeft, inventory->renderCacheTop);
                }
            }
        }
        for(int i = 0; i < INVENTORY_ROWS; ++i){
            for(int j = 0; j < INVENTORY_COLS; ++j){
                slot_render_shifted(inventory->renderCache, inventory->slots[i * INVENTORY_COLS + j], inventory->renderCacheLeft, inventory->renderCacheTop);
            }
        }
    }

    for(int index = 0; index < INVENTORY_COLS; ++index){
        slot_render_shifted(inventory->renderCache, inventory->hud->slots[index], inventory->renderCacheLeft, inventory->renderCacheTop);
    }
    inventory->renderSignature = signature;
    return 1;
}

static void inventory_render_uncached(BITMAP* scr, Inventory* inventory)
{
    if(inventory->shown){
        if(inventory->machineInventory){
            int x = SCREEN_W / 2;
            int y = (SCREEN_H - (CRAFTING_ROWS + INVENTORY_ROWS) * SLOT_SIZE) / 2 - 12;

            textout_centre_ex(
                scr,
                font,
                inventory->machineInventory->name,
                x,
                y,
                makecol(255, 255, 255),
                makecol(0, 0, 0));

            for(int i = 0; i < inventory->machineInventory->slotsCount; ++i) {
                slot_render(scr, inventory->machineInventory->slots[i]);
            }
        }else{
            for(int i = 0; i < CRAFTING_ROWS; ++i){
                for(int j = 0; j < INVENTORY_COLS; ++j){
                    slot_render(scr, inventory->crafting[i * INVENTORY_COLS + j]);
                }
            }
        }
        for(int i = 0; i < INVENTORY_ROWS; ++i){
            for(int j = 0; j < INVENTORY_COLS; ++j){
                slot_render(scr, inventory->slots[i * INVENTORY_COLS + j]);
            }
        }
    }

    hud_render(scr, inventory->hud);
}

void inventory_render(BITMAP* scr, Inventory* inventory){
    if(!scr || !inventory){
        return;
    }

    unsigned long signature = inventory_render_signature(inventory);
    int renderedUncached = 0;
    if (inventory->renderSignature != signature) {
        if (!inventory_rebuild_render_cache(inventory, signature)) {
            inventory_render_uncached(scr, inventory);
            renderedUncached = 1;
        }
    }

    if (!renderedUncached && inventory->renderCache) {
        masked_blit(
            inventory->renderCache,
            scr,
            0,
            0,
            inventory->renderCacheLeft,
            inventory->renderCacheTop,
            inventory->renderCacheWidth,
            inventory->renderCacheHeight);
    }

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

    if(inventory->machineInventory){
        for(int i = 0; i < inventory->machineInventory->slotsCount; ++i){
            Slot* slot = inventory->machineInventory->slots[i];
            Entity* sprite = slot->sprite;
            if(sprite->x <= x && x <= sprite->x + sprite->w && sprite->y <= y && y <= sprite->y + sprite->h){
                return slot;
            }
        }
    }else{
        for(int i = 0; i < INVENTORY_COLS * CRAFTING_ROWS; i++){
            Slot* slot = inventory->crafting[i];
            Entity* sprite = slot->sprite;
            if(sprite->x <= x && x <= sprite->x + sprite->w && sprite->y <= y && y <= sprite->y + sprite->h){
                return slot;
            }
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

void inventory_click(ItemRegistry* itemReg, Inventory* inventory, int x, int y)
{
    if (!inventory)
        return;

    Slot* slot = inventory_get_slot_from_coords(inventory, x, y);

    if (inventory->selectedSlot)
    {
        Slot* selected = inventory->selectedSlot;

        if (slot && slot != selected && slot->purpose != HUD_SLOT_PURPOSE_CRAFT)
        {
            int valid = 1;

            if (selected->item)
            {
                switch (slot->purpose)
                {
                case HUD_SLOT_PURPOSE_ATTACK:
                    valid = (selected->item->function == ITEM_FUNCTION_WEAPON);
                    break;

                case HUD_SLOT_PURPOSE_MINE:
                    valid = (selected->item->function == ITEM_FUNCTION_TOOL);
                    break;

                case HUD_SLOT_PURPOSE_BUILD:
                    valid = (selected->item->function == ITEM_FUNCTION_BLOCK);
                    break;
                }
            }

            if (valid)
            {
                Item* tmp = selected->item;
                slot_set_item(selected, slot->item);
                slot_set_item(slot, tmp);
            }
        }

        slot_scale_down(selected);
        inventory->selectedSlot = NULL;
        inventory_update_crafting(itemReg, inventory);
        inventory->renderSignature = 0;
        return;
    }

    if (!slot)
        return;

    if (slot->purpose == HUD_SLOT_PURPOSE_CRAFT)
    {
        if(inventory->machineInventory){
            if(slot->item && inventory_pick_item(inventory, slot->item))
                slot_set_item(slot, NULL);
        }else{
            for(int i = 0; i < INVENTORY_COLS * CRAFTING_ROWS; ++i){
                if(slot == inventory->crafting[i] && inventory->craftingItemIds[i] >= 0){
                    if(inventory_craft_item(itemReg, inventory, inventory->craftingItemIds[i])){
                        inventory_update_crafting(itemReg, inventory);
                        inventory->renderSignature = 0;
                    }
                    break;
                }
            }
        }
        return;
    }

    inventory->selectedSlot = slot;
    slot_scale_up(slot);
}
