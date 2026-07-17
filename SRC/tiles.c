#include "tiles.h"

Tile* create_tile(TextureManager* texmgr, int tex_id){
    Tile* tile = (Tile*)malloc(sizeof(Tile));
    tile->TexID = tex_id;

    int left = 0, top = 0;
    get_tile_def(tex_id, &left, &top);
    tile->Entity = create_entity(0, 0, TILE_SIZE, TILE_SIZE);

    switch(tex_id & 0xF000){
        case TILE_BLOCK:
            add_sprite_to_entity(texmgr, tile->Entity, "ASSETS/TEXTURES/ground.pcx", left, top);
            break;
        case TILE_OVERLAY:
            add_sprite_to_entity(texmgr, tile->Entity, "ASSETS/TEXTURES/overlays.pcx", left, top);
            break;
        default:
            break;
    }

    return tile;
}

static int get_block_def(int id, int* left, int* top){
    switch(id){
    case BLOCK_BARELAND:
        *left = 0;
        *top = 0;

        break;
    case BLOCK_STONE:
        *left = 16;
        *top = 0;

        break;
    case BLOCK_GRASS:
        *left = 32;
        *top = 0;

        break;
    case BLOCK_WATER:
        *left = 48;
        *top = 0;

        break;
    case BLOCK_SAND:
        *left = 0;
        *top = 16;

        break;
    case BLOCK_ORE_COPPER:
        *left = 16;
        *top = 16;

        break;
    case BLOCK_ORE_IRON:
        *left = 32;
        *top = 16;

        break;
    case BLOCK_ORE_COAL:
        *left = 48;
        *top = 16;

        break;
    default:
        return 0;
    }

    return 1;
}

static int get_overlay_def(int id, int* left, int* top)
{
    switch (id)
    {
    case OVERLAY_CONVAYER_BELT_STRAIGHT: *left =  0; *top =  0; break;
    case OVERLAY_CONVAYER_BELT_TURN:     *left = 16; *top =  0; break;

    case OVERLAY_MINE:                   *left = 32; *top =  0; break;

    case OVERLAY_FURNACE_OFF:            *left = 48; *top =  0; break;
    case OVERLAY_FURNACE_ON:             *left =  0; *top = 16; break;

    case OVERLAY_CRAFTER_HEAD_OFF:       *left = 16; *top = 16; break;
    case OVERLAY_CRAFTER_HEAD_ON:        *left = 32; *top = 16; break;

    case OVERLAY_CRAFTER_MODULE_HM:      *left = 48; *top = 16; break;
    case OVERLAY_CRAFTER_MODULE_MM:      *left =  0; *top = 32; break;
    case OVERLAY_CRAFTER_MODULE_H:       *left = 16; *top = 32; break;
    case OVERLAY_CRAFTER_MODULE_M:       *left = 32; *top = 32; break;

    case OVERLAY_SPLITTER_ALL:           *left = 48; *top = 32; break;
    case OVERLAY_SPLITTER_T:             *left =  0; *top = 48; break;
    case OVERLAY_SPLITTER_I:             *left = 16; *top = 48; break;
    case OVERLAY_SPLITTER_L:             *left = 32; *top = 48; break;

    case OVERLAY_CHEST:                  *left = 48; *top = 48; break;

    default:
        return 0;
    }

    return 1;
}

int get_tile_def(int id, int* left, int* top){
    switch(id & 0xF000){
        case TILE_BLOCK:
            return get_block_def(id, left, top);
        case TILE_OVERLAY:
            return get_overlay_def(id, left, top);
    }

    return 0;
}
