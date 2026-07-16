#include "player.h"
#include "log.h"
#include <time.h>
#include <math.h>

Player* player_create(ItemRegistry* itemReg, TextureManager* texmgr){
    Player* player = (Player*)malloc(sizeof(Player));
    player->x = 0;
    player->y = 0;
    player->speed = 5;
    player->rot = M_PI / 2;
    player->state = PLAYER_IDLE;
    player->entity = create_entity(0, 0, 16, 16);
    player->entity->scale = 2.0;

    player->vp.Left = -SCREEN_W / 2.0;
    player->vp.Top = -SCREEN_H / 2.0;
    player->vp.Width = SCREEN_W;
    player->vp.Height = SCREEN_H;

    player->inventory = inventory_create(itemReg, texmgr);
    player_select_hud_slot(player, HUD_SLOT_PURPOSE_ATTACK);

    return player;
}

void player_destroy(Player* player){
    if(!player){
        return;
    }
    destroy_entity(player->entity);
    inventory_destroy(player->inventory);
    free(player);
}

void player_update(TextureManager* texmgr, Player* player){
    if(!player){
        return;
    }

    int walkState = 0;
    if(player->walkingState > 0){
        if(player->walkingState / 10 == 0){
            walkState = 1;
            player->walkingState++;
        }else if(player->walkingState / 10 == 1){
            walkState = 2;
            player->walkingState++;
        }else{
            walkState = 0;
            player->walkingState = 0;
        }
    }

    int texLeft, texTop;
    player_get_texcoords(player->state, walkState, &texLeft, &texTop);
    add_sprite_to_entity(texmgr, player->entity, "ASSETS/TEXTURES/entity.pcx", texLeft, texTop);
    player->entity->x = player->x;
    player->entity->y = player->y;
    player->entity->angle = (player->rot / M_PI) * 180.0;
    player->vp.Left = player->x - SCREEN_W / 2.0;
    player->vp.Top = player->y - SCREEN_H / 2.0;
}

void player_look_at(Player* player, int wx, int wy){
    if(!player){
        return;
    }

    player->rot = atan2(wy - player->y, wx - player->x);
    if(isnan(player->rot)){
        player->rot = M_PI / 2;
    }
}

void player_move(Player* player, int dx, int dy) {
    if (!player) {
        return;
    }

    if (dx == 0 && dy == 0) {
        return;
    }

    double angle = player->rot;
    double speed = player->speed;

    double move_x = cos(angle) * dy;
    double move_y = sin(angle) * dy;

    move_x += -sin(angle) * dx;
    move_y += cos(angle) * dx;

    double length = sqrt(move_x * move_x + move_y * move_y);
    if (length > 0.0) {
        move_x = (move_x / length) * speed;
        move_y = (move_y / length) * speed;
    }

    player->x += (int)(move_x + 0.5);
    player->y += (int)(move_y + 0.5);
    log_debug("Player coords: (%d, %d)", player->x, player->y);

    if(player->walkingState == 0){
        player->walkingState = 1;
    }
}

void player_render(BITMAP* scr, Player* player){
    if(!player){
        return;
    }

    render_entity(scr, player->entity, &player->vp);
    inventory_render(scr, player->inventory);
}

void player_get_texcoords(int state, int walkingState, int* left, int* top){
    *top = state * 16;
    *left = walkingState * 16;
}

void player_select_hud_slot(Player* player, int slotIndex){
    if(!player){
        return;
    }

    if(player->inventory->shown){
        return;
    }

    hud_select_slot(player->inventory->hud, slotIndex);

    Slot* selectedSlot = inventory_get_selected_slot(player->inventory);
    if(!selectedSlot){
        return;
    }

    player->state = PLAYER_IDLE;
    if(selectedSlot->purpose == HUD_SLOT_PURPOSE_ATTACK){
        if(item_has_function(selectedSlot->item, ITEM_FUNCTION_WEAPON)){
            player->state = PLAYER_ATTACK;
        }
    }else if(selectedSlot->purpose == HUD_SLOT_PURPOSE_MINE){
        if(item_has_function(selectedSlot->item, ITEM_FUNCTION_TOOL)){
            player->state = PLAYER_MINE;
        }
    }else{
        if(item_has_function(selectedSlot->item, ITEM_FUNCTION_BLOCK)){
            player->state = PLAYER_BUILD;
        }
    }
}

void player_toggle_inventory(ItemRegistry* itemReg, Player* player){
    if(!player){
        return;
    }

    if(player->inventory->shown == 1){
        inventory_hide(player->inventory);
    }else{
        inventory_show(itemReg, player->inventory);
    }
}
