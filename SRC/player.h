#ifndef PLAYER_H
#define PLAYER_H
#include "entity.h"
#include "structs.h"
#include "map.h"
#include "hud.h"
#include "machine.h"

#define PLAYER_IDLE 0x0000
#define PLAYER_ATTACK 0x0001
#define PLAYER_MINE 0x0002
#define PLAYER_BUILD 0x0003

#define PLAYER_MINING_RADIUS 5 * TILE_SIZE

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
    Entity* machinePreview;
    Entity* machinePreviewSecondary;
    int machinePreviewOverlayId;
    int machinePreviewCanPlace;
    int machinePreviewSecondaryCanPlace;
    int machinePreviewRotation;
    int machinePreviewStep;

    int miningX;
    int miningY;

    int hoveredMachineX;
    int hoveredMachineY;
    int hasHoveredMachine;
} Player;

Player* player_create(ItemRegistry* itemReg, TextureManager* texmgr);
void player_destroy(Player* player);
void player_update(TextureManager* texmgr, Player* player);
void player_move(Player* player, int dx, int dy);
void player_render(BITMAP* scr, Player* player);

void player_mouse_action(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player, int x, int y, int button, int shift);
void player_mouse_move_action(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player, int x, int y);

void player_get_texcoords(int state, int walkingState, int* left, int* top);

void player_select_hud_slot(ItemRegistry* itemReg, Player* player, int slotIndex);
void player_toggle_inventory(ItemRegistry* itemReg, Player* player);
void player_cancel(Player* player);
void player_pick_drop_items(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, Player* player);
void player_rotate_preview(Player* player);

void player_open_machine(ItemRegistry* itemReg, Map* map, Player* player);

#endif
