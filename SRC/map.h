#ifndef MAP_H
#define MAP_H
#include "tiles.h"
#include "item.h"
#include "machine.h"

#define CHUNK_SIZE 16
#define MAX_ZINDEX 4

#define ZINDEX_GROUND 0
#define ZINDEX_ORES 1
#define ZINDEX_DECORATION 2
#define ZINDEX_OVERLAY 3


typedef struct {
    int X;
    int Y;

    Tile** Tiles[MAX_ZINDEX];
} Chunk;

typedef struct{
    int X;
    int Y;

    Item** items;
    int itemCount;
} DroppedItems;

typedef struct {
    Chunk** Chunks;
    int chunkCount;
    Chunk** chunkBuckets;
    int chunkBucketCount;

    DroppedItems** droppedIems;
    int droppedItemCount;

    Machine** machines;
    int machineCount;
} Map;

Tile* get_tile(Map* map, int X, int Y, int zIndex);
void set_tile(Map* map, Tile* tile, int X, int Y, int zIndex);

Map* create_map();
void render_map(BITMAP* scr, Map* map, Box* vp);
void destroy_map(Map* map);
void map_update(Map* map);

void map_drop_item(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, int wx, int wy, int itemId, int amount);
DroppedItems* map_release_dropped_items(Map* map, int wx, int wy);

Machine* map_get_machine(Map* map, int x, int y);
int map_can_place_machine(Map* map, int x, int y);
int map_place_machine(TextureManager* texmgr, Map* map, int x, int y, int overlayId, int rotation);
void map_remove_machine(Map* map, int x, int y);

Chunk* get_chunk(Map* map, int chunkX, int chunkY);
Chunk* create_chunk(TextureManager* texmgr, Map* map, int chunkX, int chunkY);
int is_chunk_in_vp(Chunk* chunk, Box* vp);

#endif // MAP_H
