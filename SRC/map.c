#include "map.h"
#include <stdio.h>
#include "log.h"
#include "utils.h"
#include "zlib.h"
#include "perlin.h"
#include "alwrap.h"

Tile* get_tile(Map* map, int X, int Y, int zIndex) {
    if(!map) return NULL;

    int chunkX = div_floor(X, CHUNK_SIZE);
    int chunkY = div_floor(Y, CHUNK_SIZE);

    Chunk* chunk = get_chunk(map, chunkX, chunkY);
    if(!chunk || !chunk->Tiles) return NULL;

    int tileX = mod_floor(X, CHUNK_SIZE);
    int tileY = mod_floor(Y, CHUNK_SIZE);

    return chunk->Tiles[zIndex][tileY * CHUNK_SIZE + tileX];
}

void set_tile(Map* map, Tile* tile, int X, int Y, int zIndex){
    if(!map || !tile || !tile->Entity) return;

    int chunkX = div_floor(X, CHUNK_SIZE);
    int chunkY = div_floor(Y, CHUNK_SIZE);

    Chunk* chunk = get_chunk(map, chunkX, chunkY);

    if(!chunk){
        return;
    }

    int tileX = mod_floor(X, CHUNK_SIZE);
    int tileY = mod_floor(Y, CHUNK_SIZE);
    int idx   = tileY * CHUNK_SIZE + tileX;

    if (chunk->Tiles[zIndex][idx]) {
        free(chunk->Tiles[zIndex][idx]);
    }

    tile->Entity->x = X * TILE_SIZE;
    tile->Entity->y = Y * TILE_SIZE;
    chunk->Tiles[zIndex][idx] = tile;
}

Map* create_map(){
    Map* map = (Map*)malloc(sizeof(Map));
    map->chunkCount = 0;
    map->Chunks = NULL;

    return map;
}

void render_map(BITMAP* scr, Map* map, Box* vp){
    if(!map){
        return;
    }

    for(int i = 0; i < map->chunkCount; i++){
        Chunk* chunk = map->Chunks[i];
        if(is_chunk_in_vp(chunk, vp) == 0){
            continue;
        }

        for(int j = 0; j < MAX_ZINDEX; j++){
            for(int k = 0; k < CHUNK_SIZE * CHUNK_SIZE; k++){
                if(!chunk->Tiles[j][k])
                    continue;

                render_entity(scr, chunk->Tiles[j][k]->Entity, vp);
            }
        }
    }
}

void destroy_map(Map* map){
    if(!map){
        return;
    }
    for(int i = 0; i < map->chunkCount; i++){
        for(int j = 0; j < MAX_ZINDEX; j++){
            for(int k = 0; k < CHUNK_SIZE * CHUNK_SIZE; k++){
                if(!map->Chunks[i]->Tiles[j][k])
                    continue;
                destroy_entity(map->Chunks[i]->Tiles[j][k]->Entity);

                free(map->Chunks[i]->Tiles[j]);
            }
        }
        free(map->Chunks[i]);
    }
    free(map->Chunks);
    free(map);
}

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

static int ensure_chunk_tiles(TextureManager* texmgr, Chunk* chunk) {
    if (chunk->Tiles[0])
        return 1;

    for (int z = 0; z < MAX_ZINDEX; z++) {
        chunk->Tiles[z] = calloc(CHUNK_SIZE * CHUNK_SIZE, sizeof(Tile*));
        if (!chunk->Tiles[z])
            return 0;
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

            Tile* ground_tile = create_tile(texmgr, tile);

            if (ground_tile && ground_tile->Entity) {
                ground_tile->Entity->x = wx * TILE_SIZE;
                ground_tile->Entity->y = wy * TILE_SIZE;

                unsigned int hash = (unsigned int)(wx * 73856093 ^ wy * 19349663);
                int rotations = hash % 4;

                if (tile == BLOCK_GRASS || tile == BLOCK_STONE || tile == BLOCK_BARELAND || tile == BLOCK_SAND) {
                    for (int r = 0; r < rotations; r++) {
                        entity_rotate_right(ground_tile->Entity);
                    }
                }

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
                        ore_tile->Entity->x = wx * TILE_SIZE;
                        ore_tile->Entity->y = wy * TILE_SIZE;
                        if(ground_tile && ground_tile->Entity){
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
        free(chunk);
        map->chunkCount--;
        return NULL;
    }

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
