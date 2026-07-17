#include "slot.h"

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

void slot_scale_up(Slot* slot){
    if(!slot || slot->sprite->scale == 1.25){
        return;
    }

    slot->sprite->x -= 2;
    slot->sprite->y -= 2;
    slot->sprite->scale = 1.25;
}

void slot_scale_down(Slot* slot){
    if(!slot || slot->sprite->scale == 1.0){
        return;
    }

    slot->sprite->x += 2;
    slot->sprite->y += 2;
    slot->sprite->scale = 1.0;
}
