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
    int lastMoveFrame;
} DroppedItems;

typedef int (*DroppedItemFilter)(Item* item, void* context);

typedef struct Map {
    Chunk** Chunks;
    int chunkCount;
    Chunk** chunkBuckets;
    int chunkBucketCount;

    DroppedItems** droppedIems;
    int droppedItemCount;

    Machine** machines;
    int machineCount;

    int frame;
} Map;

Tile* get_tile(Map* map, int X, int Y, int zIndex);
void set_tile(Map* map, Tile* tile, int X, int Y, int zIndex);

Map* create_map();
void render_map(BITMAP* scr, Map* map, Box* vp);
void destroy_map(Map* map);
void map_update(Map* map);

void map_drop_item(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, int wx, int wy, int itemId, int amount);
int map_has_dropped_items(Map* map, int wx, int wy);
DroppedItems* map_release_dropped_items(Map* map, int wx, int wy);
int map_place_dropped_items(Map* map, DroppedItems* items, int wx, int wy);
int map_place_dropped_item(Map* map, Item* item, int wx, int wy);
Item* map_take_dropped_item(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, int wx, int wy, DroppedItemFilter filter, void* context);

Machine* map_get_machine(Map* map, int x, int y);
int map_can_place_machine(Map* map, int x, int y);
int map_place_machine(TextureManager* texmgr, ItemRegistry* itemReg, Map* map, int x, int y, int overlayId, int rotation);
void map_remove_machine(Map* map, int x, int y);

Chunk* get_chunk(Map* map, int chunkX, int chunkY);
Chunk* create_chunk(TextureManager* texmgr, Map* map, int chunkX, int chunkY);
int is_chunk_in_vp(Chunk* chunk, Box* vp);

#endif // MAP_H
