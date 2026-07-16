#include "game.h"
#include "alwrap.h"
#include "entity.h"
#include "map.h"
#include "log.h"
#include "utils.h"
#include "player.h"

#define KEY_PRESSED(k) (key[(k)] && !prev_key[(k)])

static void ensure_visible_chunks(TextureManager* texmgr, Map* map, const Box* vp)
{
    int firstChunkX = div_floor(vp->Left, CHUNK_SIZE * TILE_SIZE);
    int firstChunkY = div_floor(vp->Top, CHUNK_SIZE * TILE_SIZE);

    int lastChunkX = div_floor(vp->Left + vp->Width  - 1, CHUNK_SIZE * TILE_SIZE);
    int lastChunkY = div_floor(vp->Top  + vp->Height - 1, CHUNK_SIZE * TILE_SIZE);

    for (int cy = firstChunkY; cy <= lastChunkY; cy++) {
        for (int cx = firstChunkX; cx <= lastChunkX; cx++) {
            if (!get_chunk(map, cx, cy))
                create_chunk(texmgr, map, cx, cy);
        }
    }
}

BITMAP* back_buffer = NULL;
void run_game(){
    back_buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!back_buffer) {
        log_debug("Failed to create back buffer!\n");
        return;
    }

    show_mouse(screen);
    set_mouse_range(0, 0, SCREEN_W - 1, SCREEN_H - 1);

    TextureManager* texmgr = create_texture_manager();
    ItemRegistry* itemReg = item_registry_create();

    Player* player = player_create(itemReg, texmgr);
    if (!player) {
        log_debug("Failed to create player!\n");
        return;
    }

    Map* map = create_map();
    ensure_visible_chunks(texmgr, map, &player->vp);

    int event_delay = 1;
    int event_timer = 0;
    int refresh = 1;

    char prev_key[KEY_MAX];
    memset(prev_key, 0, sizeof(prev_key));
    position_mouse(SCREEN_W / 2, SCREEN_H / 2);
    int prev_mouse_b = 0;

    while (!key[KEY_ESC]) {
        poll_mouse();

        if (event_timer >= event_delay) {
            int mouse_world_x = mouse_x + player->vp.Left;
            int mouse_world_y = mouse_y + player->vp.Top;
            player_look_at(player, mouse_world_x, mouse_world_y);
            int dx_input = 0;
            int dy_input = 0;

            if (key[KEY_W]) dy_input += 1;
            if (key[KEY_S]) dy_input -= 1;
            if (key[KEY_D]) dx_input += 1;
            if (key[KEY_A]) dx_input -= 1;

            if (KEY_PRESSED(KEY_1)) player_select_hud_slot(player, 0);
            if (KEY_PRESSED(KEY_2)) player_select_hud_slot(player, 1);
            if (KEY_PRESSED(KEY_3)) player_select_hud_slot(player, 2);
            if (KEY_PRESSED(KEY_4)) player_select_hud_slot(player, 3);
            if (KEY_PRESSED(KEY_5)) player_select_hud_slot(player, 4);
            if (KEY_PRESSED(KEY_6)) player_select_hud_slot(player, 5);

            if (KEY_PRESSED(KEY_E))
                player_toggle_inventory(itemReg, player);

            if (KEY_PRESSED(KEY_R))
                player_pick_items(map, player);

            if (dx_input != 0 || dy_input != 0) {
                player_move(player, dx_input, dy_input);

                ensure_visible_chunks(texmgr, map, &player->vp);
            }

            if (mouse_b != prev_mouse_b)
            {
                player_mouse_action(itemReg, texmgr, map, player, mouse_x, mouse_y, mouse_b);
                prev_mouse_b = mouse_b;
            }

            player_update(texmgr, player);
            refresh = 1;
            event_timer = 0;
            memcpy(prev_key, key, sizeof(prev_key));
        } else {
            event_timer++;
        }

        if (refresh != 0) {
            clear_to_color(back_buffer, makecol(0, 0, 0));

            render_map(back_buffer, map, &player->vp);
            player_render(back_buffer, player);

            vsync();

            scare_mouse();
            blit(back_buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            unscare_mouse();

            refresh = 0;
        }
    }

    player_destroy(player);
    destroy_map(map);
    destroy_texture_manager(texmgr);
    destroy_bitmap(back_buffer);
    show_mouse(NULL);
}
