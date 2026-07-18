#include "player.h"
#include "log.h"
#include "utils.h"
#include <time.h>
#include <math.h>

static int player_tile_x(Player* player)
{
    return div_floor(player->x, TILE_SIZE);
}

static int player_tile_y(Player* player)
{
    return div_floor(player->y, TILE_SIZE);
}

static Slot* player_get_selected_machine_slot(ItemRegistry* itemReg, Player* player, int* overlayId)
{
    if (!itemReg || !player || player->state != PLAYER_BUILD)
        return NULL;

    Slot* slot = inventory_get_selected_slot(player->inventory);
    if (!slot || !slot->item)
        return NULL;

    int placedAs = itemReg->info[slot->item->itemId].placedAs;

    if (overlayId)
        *overlayId = placedAs;

    return slot;
}

static void player_clear_machine_preview(Player* player)
{
    if (!player)
        return;

    player->machinePreviewCanPlace = 0;
    player->machinePreviewOverlayId = -1;

    if (player->machinePreview) {
        destroy_entity(player->machinePreview);
        player->machinePreview = NULL;
    }
}

static int player_can_place_machine_at(Map* map, Player* player, int tx, int ty)
{
    if (!map || !player)
        return 0;

    if (tx == player_tile_x(player) && ty == player_tile_y(player))
        return 0;

    return map_can_place_machine(map, tx, ty);
}

static void player_update_machine_preview(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player, int mouseX, int mouseY)
{
    int overlayId = -1;
    Slot* slot = player_get_selected_machine_slot(itemReg, player, &overlayId);

    if (!slot) {
        player_clear_machine_preview(player);
        return;
    }

    if (!player->machinePreview || player->machinePreviewOverlayId != overlayId) {
        player_clear_machine_preview(player);
        player->machinePreview = get_preview_entity(texmgr, overlayId);
        if (!player->machinePreview)
            return;
        player->machinePreviewOverlayId = overlayId;
    }

    int wx = mouseX + player->vp.Left;
    int wy = mouseY + player->vp.Top;
    int tx = div_floor(wx, TILE_SIZE);
    int ty = div_floor(wy, TILE_SIZE);

    player->machinePreview->x = tx * TILE_SIZE;
    player->machinePreview->y = ty * TILE_SIZE;
    player->machinePreview->angle = player->machinePreviewRotation;
    player->machinePreviewCanPlace = player_can_place_machine_at(map, player, tx, ty);
}

static void player_consume_selected_item(Player* player, Slot* slot)
{
    if (!player || !slot || !slot->item)
        return;

    slot->item->amount--;

    if (slot->item->amount <= 0) {
        item_destroy(slot->item);
        slot->item = NULL;
        player->state = PLAYER_IDLE;
        player->inventory->hud->selectedInfo = NULL;
        player_clear_machine_preview(player);
    }
}

Player* player_create(ItemRegistry* itemReg, TextureManager* texmgr){
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player)
        return NULL;

    player->x = 0;
    player->y = 0;
    player->miningX = 0;
    player->miningY = 0;
    player->speed = 5;
    player->rot = M_PI / 2;
    player->state = PLAYER_IDLE;
    player->walkingState = 0;
    player->machinePreview = NULL;
    player->machinePreviewOverlayId = -1;
    player->machinePreviewCanPlace = 0;
    player->machinePreviewRotation = 0;
    player->entity = create_entity(0, 0, 16, 16);
    if (!player->entity) {
        free(player);
        return NULL;
    }
    player->entity->scale = 2.0;

    player->vp.Left = -SCREEN_W / 2.0;
    player->vp.Top = -SCREEN_H / 2.0;
    player->vp.Width = SCREEN_W;
    player->vp.Height = SCREEN_H;

    player->inventory = inventory_create(itemReg, texmgr);
    if (!player->inventory) {
        destroy_entity(player->entity);
        free(player);
        return NULL;
    }
    player_select_hud_slot(itemReg, player, HUD_SLOT_PURPOSE_ATTACK);

    return player;
}

void player_destroy(Player* player){
    if(!player){
        return;
    }
    destroy_entity(player->machinePreview);
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

    if(player->walkingState == 0){
        player->walkingState = 1;
    }
}

void player_render(BITMAP* scr, Player* player){
    if(!player){
        return;
    }


    if(player->state == PLAYER_MINE){
        int sx = player->miningX * TILE_SIZE - player->vp.Left;
        int sy = player->miningY * TILE_SIZE - player->vp.Top;
        rect(scr, sx - 1, sy - 1, sx + TILE_SIZE, sy + TILE_SIZE, makecol(0, 0, 0));
    }
    else if(player->state == PLAYER_BUILD){
        if(player->machinePreview && player->machinePreviewCanPlace){
            int sx = player->machinePreview->x - player->vp.Left;
            int sy = player->machinePreview->y - player->vp.Top;
            rect(scr, sx - 1, sy - 1, sx + TILE_SIZE, sy + TILE_SIZE, makecol(0, 0, 0));
            render_entity(scr, player->machinePreview, &player->vp);
        }
    }

    render_entity(scr, player->entity, &player->vp);
    inventory_render(scr, player->inventory);
}

void player_get_texcoords(int state, int walkingState, int* left, int* top){
    *top = state * 16;
    *left = walkingState * 16;
}

void player_select_hud_slot(ItemRegistry* itemReg, Player* player, int slotIndex){
    if(!player){
        return;
    }

    if(player->inventory->shown){
        return;
    }

    hud_select_slot(itemReg, player->inventory->hud, slotIndex);
    player_clear_machine_preview(player);

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
        inventory_show(itemReg, player->inventory, NULL);
        player_clear_machine_preview(player);
    }
}

void player_cancel(Player* player){
    if(!player){
        return;
    }

    if(player->inventory->shown == 1){
        inventory_hide(player->inventory);
        player_clear_machine_preview(player);
    }
}

void player_mouse_action(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player, int x, int y, int button){
    if(!map || !player){
        return;
    }

    if(player->inventory->shown){
        if(button & 1){
            inventory_click(itemReg, player->inventory, x, y);
        }
    }else{
        int wx = x + player->vp.Left;
        int wy = y + player->vp.Top;
        if(button & 1){
            if(player->state == PLAYER_MINE){
                if((player->x - wx) * (player->x - wx) + (player->y - wy) * (player->y - wy) <= PLAYER_MINING_RADIUS * PLAYER_MINING_RADIUS){
                    int tx = div_floor(wx, TILE_SIZE);
                    int ty = div_floor(wy, TILE_SIZE);

                    for(int z = 3; z >= 0; --z){
                        Tile* tile = get_tile(map, tx, ty, z);
                        if(!tile){
                            continue;
                        }

                        for(int i = 0; i < ITEM_COUNT; i++){
                            if((tile->TexID & 0xFFF0) == itemReg->info[i].aquiredFrom){
                                map_drop_item(itemReg, texmgr, map, tx, ty, i, 1);
                                if((tile->TexID & 0xF000) == TILE_OVERLAY){
                                    map_remove_machine(map, tx, ty);
                                    set_tile(map, NULL, tx, ty, z);
                                }
                                return;
                            }
                        }
                    }
                }
            }else if(player->state == PLAYER_BUILD){
                if((player->x - wx) * (player->x - wx) + (player->y - wy) * (player->y - wy) <= PLAYER_MINING_RADIUS * PLAYER_MINING_RADIUS){
                    int overlayId = -1;
                    Slot* slot = player_get_selected_machine_slot(itemReg, player, &overlayId);
                    int tx = div_floor(wx, TILE_SIZE);
                    int ty = div_floor(wy, TILE_SIZE);

                    if(slot && player_can_place_machine_at(map, player, tx, ty)){
                        if(map_place_machine(texmgr, itemReg, map, tx, ty, overlayId, player->machinePreviewRotation)){
                            player->machinePreviewCanPlace = 0;
                            player_consume_selected_item(player, slot);
                        }
                    }
                }
            }
        }
    }
}

void player_mouse_move_action(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player, int x, int y){
    if(!player){
        return;
    }
    int wx = x + player->vp.Left;
    int wy = y + player->vp.Top;

    player->rot = atan2(wy - player->y, wx - player->x);
    if(isnan(player->rot)){
        player->rot = M_PI / 2;
        return;
    }

    if(player->state == PLAYER_MINE){
        player->miningX = div_floor(wx, TILE_SIZE);
        player->miningY = div_floor(wy, TILE_SIZE);
    }
    else {
        if(player->inventory->shown){
            inventory_hover(itemReg, player->inventory, x, y);
            player_clear_machine_preview(player);
        }else{
            player_update_machine_preview(itemReg, texmgr, map, player, x, y);
        }
    }
}

void player_pick_drop_items(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player){
    if(!map || !player){
        return;
    }

    int tx = player->x / TILE_SIZE;
    int ty = player->y / TILE_SIZE;
    if(player->inventory->shown){
        Slot* slot = player->inventory->selectedSlot;
        if(slot && slot->item){
            map_drop_item(itemReg, texmgr, map, tx, ty, slot->item->itemId, slot->item->amount);
            item_destroy(slot->item);
            slot->item = NULL;
        }
        slot_scale_down(slot);
        player->inventory->selectedSlot = NULL;
    }else{
        for(int x = tx - 1; x <= tx + 1; x++){
            for(int y = ty - 1; y <= ty + 1; y++){
                DroppedItems* items = map_release_dropped_items(map, x, y);
                if(!items){
                    continue;
                }

                for(int i = 0; i < items->itemCount; i++){
                    if(!inventory_pick_item(player->inventory, items->items[i])){
                        item_destroy(items->items[i]);
                    }
                }

                free(items->items);
                free(items);
            }
        }
    }
}

void player_rotate_preview(Player* player){
    if(!player){
        return;
    }

    if(player->machinePreview){
        player->machinePreviewRotation = (player->machinePreviewRotation + 90) % 360;
        player->machinePreview->angle = player->machinePreviewRotation;
    }
}

void player_open_machine(ItemRegistry* itemReg, Map* map, Player* player){
    if(!itemReg || !map || !player){
        return;
    }

    int tx = player->x / TILE_SIZE;
    int ty = player->y / TILE_SIZE;
        for(int x = tx - 1; x <= tx + 1; x++){
            for(int y = ty - 1; y <= ty + 1; y++){
                Machine* machine = map_get_machine(map, x, y);
                if(!machine){
                    continue;
                }

                if(machine->inventory){
                    inventory_show(machine->itemReg, player->inventory, machine->inventory);
                    return;
                }
            }
        }

}
