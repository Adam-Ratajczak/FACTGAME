#include "map.h"
#include <stdio.h>
#include "log.h"
#include "utils.h"
#include "zlib.h"
#include "perlin.h"
#include "alwrap.h"

static unsigned int chunk_hash(int chunkX, int chunkY)
{
    return (unsigned int)(chunkX * 73856093u) ^ (unsigned int)(chunkY * 19349663u);
}

static void map_disable_chunk_buckets(Map* map)
{
    free(map->chunkBuckets);
    map->chunkBuckets = NULL;
    map->chunkBucketCount = 0;
}

static int map_rebuild_chunk_buckets(Map* map, int bucketCount)
{
    Chunk** buckets = calloc(bucketCount, sizeof(Chunk*));
    if (!buckets)
        return 0;

    for (int i = 0; i < map->chunkCount; ++i) {
        Chunk* chunk = map->Chunks[i];
        unsigned int index = chunk_hash(chunk->X, chunk->Y) & (bucketCount - 1);

        while (buckets[index])
            index = (index + 1) & (bucketCount - 1);

        buckets[index] = chunk;
    }

    free(map->chunkBuckets);
    map->chunkBuckets = buckets;
    map->chunkBucketCount = bucketCount;
    return 1;
}

static int map_ensure_chunk_buckets(Map* map)
{
    int wanted = 64;
    while (wanted < (map->chunkCount + 1) * 2)
        wanted *= 2;

    if (map->chunkBucketCount >= wanted)
        return 1;

    return map_rebuild_chunk_buckets(map, wanted);
}

static int map_insert_chunk_bucket(Map* map, Chunk* chunk)
{
    if (!map_ensure_chunk_buckets(map))
        return 0;

    unsigned int index = chunk_hash(chunk->X, chunk->Y) & (map->chunkBucketCount - 1);

    while (map->chunkBuckets[index]) {
        Chunk* existing = map->chunkBuckets[index];
        if (existing->X == chunk->X && existing->Y == chunk->Y)
            return 1;

        index = (index + 1) & (map->chunkBucketCount - 1);
    }

    map->chunkBuckets[index] = chunk;
    return 1;
}

Tile* get_tile(Map* map, int X, int Y, int zIndex) {
    if(!map || zIndex < 0 || zIndex >= MAX_ZINDEX) return NULL;

    int chunkX = div_floor(X, CHUNK_SIZE);
    int chunkY = div_floor(Y, CHUNK_SIZE);

    Chunk* chunk = get_chunk(map, chunkX, chunkY);
    if(!chunk || !chunk->Tiles[zIndex]) return NULL;

    int tileX = mod_floor(X, CHUNK_SIZE);
    int tileY = mod_floor(Y, CHUNK_SIZE);

    return chunk->Tiles[zIndex][tileY * CHUNK_SIZE + tileX];
}

void set_tile(Map* map, Tile* tile, int X, int Y, int zIndex){
    if(!map || zIndex < 0 || zIndex >= MAX_ZINDEX) return;
    if(tile && !tile->Entity) return;

    int chunkX = div_floor(X, CHUNK_SIZE);
    int chunkY = div_floor(Y, CHUNK_SIZE);

    Chunk* chunk = get_chunk(map, chunkX, chunkY);

    if(!chunk){
        return;
    }

    int tileX = mod_floor(X, CHUNK_SIZE);
    int tileY = mod_floor(Y, CHUNK_SIZE);
    int idx   = tileY * CHUNK_SIZE + tileX;

    destroy_tile(chunk->Tiles[zIndex][idx]);

    if (tile) {
        tile->Entity->x = X * TILE_SIZE;
        tile->Entity->y = Y * TILE_SIZE;
    }
    chunk->Tiles[zIndex][idx] = tile;
}

Map* create_map(){
    Map* map = (Map*)malloc(sizeof(Map));
    if (!map)
        return NULL;

    map->chunkCount = 0;
    map->Chunks = NULL;
    map->chunkBuckets = NULL;
    map->chunkBucketCount = 0;
    map->droppedItemCount = 0;
    map->droppedIems = NULL;
    map->machineCount = 0;
    map->machines = NULL;

    return map;
}

static void render_chunk(BITMAP* scr, Chunk* chunk, Box* vp)
{
    if (!chunk)
        return;

    for(int j = 0; j < MAX_ZINDEX; j++){
        if (!chunk->Tiles[j])
            continue;

        for(int k = 0; k < CHUNK_SIZE * CHUNK_SIZE; k++){
            if(!chunk->Tiles[j][k])
                continue;

            render_entity(scr, chunk->Tiles[j][k]->Entity, vp);
        }
    }
}

void render_map(BITMAP* scr, Map* map, Box* vp){
    if(!map || !vp){
        return;
    }

    int chunkPixels = CHUNK_SIZE * TILE_SIZE;
    int firstChunkX = div_floor(vp->Left, chunkPixels);
    int firstChunkY = div_floor(vp->Top, chunkPixels);
    int lastChunkX = div_floor(vp->Left + vp->Width - 1, chunkPixels);
    int lastChunkY = div_floor(vp->Top + vp->Height - 1, chunkPixels);

    for (int cy = firstChunkY; cy <= lastChunkY; ++cy) {
        for (int cx = firstChunkX; cx <= lastChunkX; ++cx) {
            render_chunk(scr, get_chunk(map, cx, cy), vp);
        }
    }

    for(int i = 0; i < map->droppedItemCount; i++){
        for(int j = 0; j < map->droppedIems[i]->itemCount; j++){
            item_render(scr, map->droppedIems[i]->items[j], vp);
        }
    }
}

void destroy_map(Map* map){
    if(!map){
        return;
    }
    for(int i = 0; i < map->chunkCount; i++){
        for(int j = 0; j < MAX_ZINDEX; j++){
            if (!map->Chunks[i]->Tiles[j])
                continue;

            for(int k = 0; k < CHUNK_SIZE * CHUNK_SIZE; k++){
                if(!map->Chunks[i]->Tiles[j][k])
                    continue;
                destroy_tile(map->Chunks[i]->Tiles[j][k]);
            }
            free(map->Chunks[i]->Tiles[j]);
        }
        free(map->Chunks[i]);
    }
    free(map->Chunks);
    free(map->chunkBuckets);

    for(int i = 0; i < map->droppedItemCount; i++){
        for(int j = 0; j < map->droppedIems[i]->itemCount; j++){
            item_destroy(map->droppedIems[i]->items[j]);
        }
        free(map->droppedIems[i]->items);
        free(map->droppedIems[i]);
    }
    free(map->droppedIems);

    for(int i = 0; i < map->machineCount; i++){
        machine_destroy(map->machines[i]);
    }
    free(map->machines);
    free(map);
}

void map_update(Map* map){
    if(!map){
        return;
    }

    for(int i = 0; i < map->machineCount; ++i){
        machine_update(map->machines[i], map);
    }
}

static int ensure_chunk_tiles(TextureManager* texmgr, Chunk* chunk) {
    if (chunk->Tiles[0])
        return 1;

    for (int z = 0; z < MAX_ZINDEX; z++) {
        chunk->Tiles[z] = calloc(CHUNK_SIZE * CHUNK_SIZE, sizeof(Tile*));
        if (!chunk->Tiles[z]) {
            for (int i = 0; i < z; ++i) {
                free(chunk->Tiles[i]);
                chunk->Tiles[i] = NULL;
            }
            return 0;
        }
    }

    const float geo_freq = 0.003f;
    const int geo_depth = 4;
    const int seed_height = 12345;
    const int seed_humidity = 54321;
    const int seed_ore = 99999;

    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            int index = y * CHUNK_SIZE + x;

            int wx = chunk->X * CHUNK_SIZE + x;
            int wy = chunk->Y * CHUNK_SIZE + y;

            float height = (perlin2d(wx, wy, geo_freq, geo_depth, seed_height) + 1.0f) * 0.5f;
            float humidity = (perlin2d(wx, wy, geo_freq, geo_depth, seed_humidity) + 1.0f) * 0.5f;

            int tile = BLOCK_BARELAND;

            if (height < 0.30f) {
                tile = BLOCK_WATER;
            }
            else if (height < 0.38f) {
                if (humidity < 0.45f) {
                    tile = BLOCK_SAND;
                } else {
                    tile = BLOCK_GRASS;
                }
            }
            else if (height < 0.75f) {
                if (humidity < 0.35f) {
                    tile = BLOCK_BARELAND;
                } else if (humidity < 0.70f) {
                    tile = BLOCK_GRASS;
                } else {
                    tile = (humidity > 0.85f) ? BLOCK_WATER : BLOCK_GRASS;
                }
            }
            else {
                tile = BLOCK_STONE;
            }

            unsigned int hash = (unsigned int)(wx * 73856093 ^ wy * 19349663);
            int rotations = hash % 4;
            int angle = 0;

            if (tile == BLOCK_GRASS || tile == BLOCK_STONE || tile == BLOCK_BARELAND || tile == BLOCK_SAND) {
                angle = rotations * 90;
            }

            Tile* ground_tile = create_tile_rotated(texmgr, tile, angle);

            if (ground_tile && ground_tile->Entity) {
                ground_tile->Entity->x = wx * TILE_SIZE;
                ground_tile->Entity->y = wy * TILE_SIZE;

                chunk->Tiles[ZINDEX_GROUND][index] = ground_tile;
            }

            if (tile == BLOCK_STONE || tile == BLOCK_BARELAND) {
                float ore_val = (perlin2d(wx, wy, 0.09f, 3, seed_ore) + 1.0f) * 0.5f;

                if (ore_val > 0.82f) {
                    int ore_type = -1;

                    int sector_size = 32;
                    int sector_x = wx >= 0 ? (wx / sector_size) : ((wx - sector_size + 1) / sector_size);
                    int sector_y = wy >= 0 ? (wy / sector_size) : ((wy - sector_size + 1) / sector_size);

                    unsigned int sector_hash = (unsigned int)(sector_x * 73856093 ^ sector_y * 19349663 ^ seed_ore);
                    int ore_roll = sector_hash % 3; // 3 types of ore: Coal, Iron, Copper

                    if (ore_roll == 0) {
                        ore_type = BLOCK_ORE_COAL;
                    }
                    else if (ore_roll == 1) {
                        if (height > 0.50f) {
                            ore_type = BLOCK_ORE_IRON;
                        } else {
                            ore_type = BLOCK_ORE_COAL;
                        }
                    }
                    else {
                        ore_type = BLOCK_ORE_COPPER;
                    }

                    if (ore_type != -1) {
                        Tile* ore_tile = create_tile(texmgr, ore_type);
                        if(ore_tile && ore_tile->Entity && ground_tile && ground_tile->Entity){
                            ore_tile->Entity->x = wx * TILE_SIZE;
                            ore_tile->Entity->y = wy * TILE_SIZE;
                            chunk->Tiles[ZINDEX_ORES][index] = ore_tile;
                        }
                    }
                }
            }
        }
    }

    log_debug("Generated chunk (%d, %d)", chunk->X, chunk->Y);
    return 1;
}

Chunk* get_chunk(Map* map, int chunkX, int chunkY) {
    if (!map)
        return NULL;

    if (map->chunkBuckets && map->chunkBucketCount > 0) {
        unsigned int index = chunk_hash(chunkX, chunkY) & (map->chunkBucketCount - 1);

        while (map->chunkBuckets[index]) {
            Chunk* chunk = map->chunkBuckets[index];
            if (chunk->X == chunkX && chunk->Y == chunkY)
                return chunk;

            index = (index + 1) & (map->chunkBucketCount - 1);
        }

        return NULL;
    }

    for (int i = 0; i < map->chunkCount; i++) {
        if (map->Chunks[i]->X == chunkX &&
            map->Chunks[i]->Y == chunkY)
            return map->Chunks[i];
    }

    return NULL;
}

Chunk* create_chunk(TextureManager* texmgr, Map* map, int chunkX, int chunkY) {
    if (!map)
        return NULL;

    Chunk** grown = realloc(map->Chunks, (map->chunkCount + 1) * sizeof(Chunk*));
    if (!grown)
        return NULL;

    map->Chunks = grown;

    Chunk* chunk = calloc(1, sizeof(Chunk));
    if (!chunk)
        return NULL;

    chunk->X = chunkX;
    chunk->Y = chunkY;

    map->Chunks[map->chunkCount++] = chunk;

    if (!ensure_chunk_tiles(texmgr, chunk)) {
        map->chunkCount--;
        map->Chunks[map->chunkCount] = NULL;
        free(chunk);
        return NULL;
    }

    if (!map_insert_chunk_bucket(map, chunk))
        map_disable_chunk_buckets(map);

    return chunk;
}

int is_chunk_in_vp(Chunk* chunk, Box* vp){
    int wx1 = chunk->X * CHUNK_SIZE * TILE_SIZE;
    int wy1 = chunk->Y * CHUNK_SIZE * TILE_SIZE;
    int wx2 = wx1 + (CHUNK_SIZE * TILE_SIZE);
    int wy2 = wy1 + (CHUNK_SIZE * TILE_SIZE);

    int vp_right = vp->Left + vp->Width;
    int vp_bottom = vp->Top + vp->Height;

    return (wx1 < vp_right && wx2 > vp->Left &&
            wy1 < vp_bottom && wy2 > vp->Top);
}

void map_drop_item(ItemRegistry* itemReg, TextureManager* texmgr, Map* map, int wx, int wy, int itemId, int amount){
    if (!map || amount <= 0)
        return;


    DroppedItems* drop = NULL;

    for (int i = 0; i < map->droppedItemCount; ++i)
    {
        if (map->droppedIems[i]->X == wx &&
            map->droppedIems[i]->Y == wy)
        {
            drop = map->droppedIems[i];
            break;
        }
    }

    if (!drop)
    {
        DroppedItems** grown = realloc(map->droppedIems, (map->droppedItemCount + 1) * sizeof(*map->droppedIems));

        if (!grown)
            return;

        map->droppedIems = grown;

        drop = calloc(1, sizeof(*drop));
        if (!drop)
            return;

        drop->X = wx;
        drop->Y = wy;

        map->droppedIems[map->droppedItemCount++] = drop;
    }

    for (int i = 0; i < drop->itemCount; ++i)
    {
        if (drop->items[i]->itemId == itemId)
        {
            drop->items[i]->amount += amount;
            return;
        }
    }

    Item** grown = realloc(drop->items, (drop->itemCount + 1) * sizeof(*drop->items));

    if (!grown)
        return;

    drop->items = grown;

    Item* item = item_create(itemReg, texmgr, itemId, amount);
    if (!item)
        return;

    item->sprite->x = wx * TILE_SIZE + 4;
    item->sprite->y = wy * TILE_SIZE + 4;
    item->inInventory = 0;

    drop->items[drop->itemCount++] = item;
    log_debug("Dropped %x at (%d, %d)", itemId, item->sprite->x, item->sprite->y);
}

DroppedItems* map_release_dropped_items(Map* map, int wx, int wy)
{
    if (!map)
        return NULL;

    for (int i = 0; i < map->droppedItemCount; ++i)
    {
        DroppedItems* drop = map->droppedIems[i];

        if (drop->X != wx || drop->Y != wy)
            continue;

        for (int j = i + 1; j < map->droppedItemCount; ++j)
            map->droppedIems[j - 1] = map->droppedIems[j];

        map->droppedItemCount--;

        if (map->droppedItemCount == 0)
        {
            free(map->droppedIems);
            map->droppedIems = NULL;
        }
        else
        {
            DroppedItems** grown = realloc(map->droppedIems, map->droppedItemCount * sizeof(*map->droppedIems));

            if (grown)
                map->droppedIems = grown;
        }

        return drop;
    }

    return NULL;
}

Machine* map_get_machine(Map* map, int x, int y)
{
    if (!map)
        return NULL;

    for (int i = 0; i < map->machineCount; ++i) {
        if (map->machines[i]->X == x && map->machines[i]->Y == y)
            return map->machines[i];
    }

    return NULL;
}

int map_can_place_machine(Map* map, int x, int y)
{
    if (!map)
        return 0;

    if (get_tile(map, x, y, ZINDEX_OVERLAY))
        return 0;

    if (map_get_machine(map, x, y))
        return 0;

    return 1;
}

int map_place_machine(TextureManager* texmgr, Map* map, int x, int y, int overlayId, int rotation)
{
    if (!texmgr || !map || !map_can_place_machine(map, x, y))
        return 0;

    if (!get_chunk(map, div_floor(x, CHUNK_SIZE), div_floor(y, CHUNK_SIZE)))
        return 0;

    Machine* machine = machine_create(texmgr, x, y, rotation, overlayId);
    if (!machine)
        return 0;

    Tile* tile = create_tile(texmgr, machine_get_tile_id(overlayId));
    if (!tile) {
        machine_destroy(machine);
        return 0;
    }
    tile->Entity->angle = rotation;

    Machine** grown = realloc(map->machines, (map->machineCount + 1) * sizeof(*map->machines));
    if (!grown) {
        destroy_tile(tile);
        machine_destroy(machine);
        return 0;
    }

    map->machines = grown;
    map->machines[map->machineCount++] = machine;
    set_tile(map, tile, x, y, ZINDEX_OVERLAY);

    return 1;
}

void map_remove_machine(Map* map, int x, int y)
{
    if (!map)
        return;

    for (int i = 0; i < map->machineCount; ++i) {
        Machine* machine = map->machines[i];

        if (machine->X != x || machine->Y != y)
            continue;

        machine_destroy(machine);

        for (int j = i + 1; j < map->machineCount; ++j)
            map->machines[j - 1] = map->machines[j];

        map->machineCount--;

        if (map->machineCount == 0) {
            free(map->machines);
            map->machines = NULL;
        } else {
            Machine** grown = realloc(map->machines, map->machineCount * sizeof(*map->machines));
            if (grown)
                map->machines = grown;
        }

        return;
    }
}
