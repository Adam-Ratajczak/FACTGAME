#ifndef ENTITY_H
#define ENTITY_H
#include "texmgr.h"
#include "structs.h"

#define TILE_SIZE 16

typedef struct
{
    int x;
    int y;
    int w;
    int h;

    BITMAP* sheet;
    int sx;
    int sy;
    int angle;
    float scale;
} Entity;

Entity* create_entity(int ex, int ey, int ew, int eh);
void add_sprite_to_entity(TextureManager* manager, Entity* entity, const char* path, int sx, int sy);
void entity_rotate_right(Entity* entity);
void destroy_entity(Entity* entity);

void render_entity(BITMAP* scr, Entity* entity, const Box* vp);

#define TILE_BLOCK 0x0000
#define TILE_OVERLAY 0x1000

#define BLOCK_BARELAND 0x0000
#define BLOCK_STONE 0x0001
#define BLOCK_GRASS 0x0002
#define BLOCK_WATER 0x0003
#define BLOCK_SAND 0x0004
#define BLOCK_ORE_COPPER 0x0010
#define BLOCK_ORE_IRON 0x0011
#define BLOCK_ORE_COAL 0x0012

#define ERROR_NOT_FOUND 0x1
#define ERROR_INVALID 0x2

int get_tile_def(int id, int* left, int* top);

#endif // ENTITY_H
