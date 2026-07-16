#ifndef MAP_H
#define MAP_H
#include "entity.h"

#define CHUNK_SIZE 16
#define MAX_ZINDEX 4

#define ZINDEX_GROUND 0
#define ZINDEX_ORES 1
#define ZINDEX_DECORATION 2

typedef struct {
    int TexID;
    Entity* Entity;
} Tile;

typedef struct {
    int X;
    int Y;

    Tile** Tiles[MAX_ZINDEX];
} Chunk;

typedef struct {
    Chunk** Chunks;
    int chunkCount;
} Map;

Tile* get_tile(Map* map, int X, int Y, int zIndex);
void set_tile(Map* map, Tile* tile, int X, int Y, int zIndex);

Map* create_map();
void render_map(BITMAP* scr, Map* map, Box* vp);
void destroy_map(Map* map);

Tile* create_tile(TextureManager* texmgr, int tex_id);
Chunk* get_chunk(Map* map, int chunkX, int chunkY);
Chunk* create_chunk(TextureManager* texmgr, Map* map, int chunkX, int chunkY);
int is_chunk_in_vp(Chunk* chunk, Box* vp);

#endif // MAP_H
