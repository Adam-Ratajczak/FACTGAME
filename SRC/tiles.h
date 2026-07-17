#ifndef TILES_H
#define TILES_H
#include "entity.h"

#define TILE_SIZE 16

#define TILE_BLOCK      0x0000
#define TILE_OVERLAY    0x1000

#define BLOCK_BARELAND      0x0000
#define BLOCK_STONE         0x0010
#define BLOCK_GRASS         0x0020
#define BLOCK_WATER         0x0030
#define BLOCK_SAND          0x0040
#define BLOCK_ORE_COPPER    0x0050
#define BLOCK_ORE_IRON      0x0060
#define BLOCK_ORE_COAL      0x0070

#define OVERLAY_CONVAYER_BELT               0x1000
#define OVERLAY_CONVAYER_BELT_STRAIGHT      0x1001
#define OVERLAY_CONVAYER_BELT_TURN          0x1002
#define OVERLAY_MINE                        0x1010
#define OVERLAY_FURNACE                     0x1020
#define OVERLAY_FURNACE_OFF                 0x1021
#define OVERLAY_FURNACE_ON                  0x1022
#define OVERLAY_CRAFTER_HEAD                0x1030
#define OVERLAY_CRAFTER_HEAD_OFF            0x1031
#define OVERLAY_CRAFTER_HEAD_ON             0x1032
#define OVERLAY_CRAFTER_MODULE              0x1040
#define OVERLAY_CRAFTER_MODULE_HM           0x1041
#define OVERLAY_CRAFTER_MODULE_MM           0x1042
#define OVERLAY_CRAFTER_MODULE_H            0x1043
#define OVERLAY_CRAFTER_MODULE_M            0x1044
#define OVERLAY_SPLITTER                    0x1050
#define OVERLAY_SPLITTER_ALL                0x1051
#define OVERLAY_SPLITTER_T                  0x1052
#define OVERLAY_SPLITTER_I                  0x1053
#define OVERLAY_SPLITTER_L                  0x1054
#define OVERLAY_CHEST                       0x1060

typedef struct {
    int TexID;
    Entity* Entity;
} Tile;

Tile* create_tile(TextureManager* texmgr, int tex_id);
Tile* create_tile_rotated(TextureManager* texmgr, int tex_id, int angle);
void destroy_tile(Tile* tile);

int get_tile_def(int id, int* left, int* top);

#endif
