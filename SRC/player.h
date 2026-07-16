#ifndef PLAYER_H
#define PLAYER_H
#include "entity.h"
#include "structs.h"
#include "map.h"
#include "hud.h"

#define PLAYER_IDLE 0x0000
#define PLAYER_ATTACK 0x0001
#define PLAYER_MINE 0x0002
#define PLAYER_BUILD 0x0003

typedef struct
{
    int x;
    int y;
    int speed;
    float rot;

    Box vp;
    Entity* entity;

    int state;
    int walkingState;

    Inventory* inventory;
} Player;

Player* player_create(ItemRegistry* itemReg, TextureManager* texmgr);
void player_destroy(Player* player);
void player_update(TextureManager* texmgr, Player* player);
void player_look_at(Player* player, int wx, int wy);
void player_move(Player* player, int dx, int dy);
void player_render(BITMAP* scr, Player* player);

void player_get_texcoords(int state, int walkingState, int* left, int* top);

void player_select_hud_slot(Player* player, int slotIndex);
void player_toggle_inventory(ItemRegistry* itemReg, Player* player);

#endif