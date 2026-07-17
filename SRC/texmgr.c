#include "texmgr.h"
#include <stdio.h>

TextureManager* create_texture_manager() {
    TextureManager* manager = (TextureManager*)malloc(sizeof(TextureManager));
    if (!manager) {
        return NULL;
    }
    manager->textures = NULL;
    manager->texture_count = 0;
    manager->regions = NULL;
    manager->region_count = 0;
    return manager;
}

void destroy_texture_manager(TextureManager* manager){
    if (manager) {
        for (int i = 0; i < manager->region_count; i++) {
            if (manager->regions[i].bitmap) {
                destroy_bitmap(manager->regions[i].bitmap);
            }
            if (manager->regions[i].file_path) {
                free(manager->regions[i].file_path);
            }
        }
        free(manager->regions);

        for (int i = 0; i < manager->texture_count; i++) {
            if (manager->textures[i].bitmap) {
                destroy_bitmap(manager->textures[i].bitmap);
            }
            if (manager->textures[i].file_path) {
                free(manager->textures[i].file_path);
            }
        }
        free(manager->textures);
        free(manager);
    }
}

static BITMAP *load_asset_bmp(const char *rel)
{
    char exe[512], full[512];
    get_executable_name(exe, sizeof(exe));
    replace_filename(full, exe, rel, sizeof(full));
    for (char *p=full; *p; ++p) if (*p=='\\') *p='/';

    errno = 0;
    BITMAP *bmp = load_bitmap(full, NULL);
    if(bmp){
        int trans_color = bmp->vtable->mask_color;
        log_debug("Loaded %s: mask color is %08X\n", rel, trans_color);
    }
    return bmp;
}

BITMAP* load_texture(TextureManager* manager, const char* file_path){
    if (!manager || !file_path) {
        return NULL;
    }

    for (int i = 0; i < manager->texture_count; i++) {
        if (strcmp(manager->textures[i].file_path, file_path) == 0) {
            return manager->textures[i].bitmap;
        }
    }

    BITMAP* bitmap = load_asset_bmp(file_path);
    if (!bitmap) {
        return NULL;
    }

    Texture* new_textures = (Texture*)realloc(manager->textures, (manager->texture_count + 1) * sizeof(Texture));
    if (!new_textures) {
        destroy_bitmap(bitmap);
        return NULL;
    }
    manager->textures = new_textures;

    manager->textures[manager->texture_count].file_path = strdup(file_path);
    manager->textures[manager->texture_count].bitmap = bitmap;
    manager->texture_count++;

    return bitmap;
}

static int normalized_angle(int angle)
{
    angle %= 360;
    if (angle < 0)
        angle += 360;
    return angle;
}

static BITMAP* create_region_bitmap(BITMAP* source, int sx, int sy, int w, int h, int angle)
{
    BITMAP* region = create_bitmap(w, h);
    if (!region)
        return NULL;

    int mask = source->vtable->mask_color;
    clear_to_color(region, mask);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int src_x = x;
            int src_y = y;

            switch (angle) {
            case 90:
                src_x = y;
                src_y = h - 1 - x;
                break;
            case 180:
                src_x = w - 1 - x;
                src_y = h - 1 - y;
                break;
            case 270:
                src_x = w - 1 - y;
                src_y = x;
                break;
            default:
                break;
            }

            int color = getpixel(source, sx + src_x, sy + src_y);
            if (color != mask)
                putpixel(region, x, y, color);
        }
    }

    return region;
}

BITMAP* load_texture_region(TextureManager* manager, const char* file_path, int sx, int sy, int w, int h, int angle)
{
    if (!manager || !file_path || w <= 0 || h <= 0)
        return NULL;

    angle = normalized_angle(angle);

    for (int i = 0; i < manager->region_count; i++) {
        TextureRegion* region = &manager->regions[i];
        if (region->sx == sx &&
            region->sy == sy &&
            region->w == w &&
            region->h == h &&
            region->angle == angle &&
            strcmp(region->file_path, file_path) == 0) {
            return region->bitmap;
        }
    }

    BITMAP* source = load_texture(manager, file_path);
    if (!source)
        return NULL;

    BITMAP* bitmap = create_region_bitmap(source, sx, sy, w, h, angle);
    if (!bitmap)
        return NULL;

    TextureRegion* grown = realloc(manager->regions, (manager->region_count + 1) * sizeof(TextureRegion));
    if (!grown) {
        destroy_bitmap(bitmap);
        return NULL;
    }

    manager->regions = grown;
    manager->regions[manager->region_count].file_path = strdup(file_path);
    if (!manager->regions[manager->region_count].file_path) {
        destroy_bitmap(bitmap);
        return NULL;
    }

    manager->regions[manager->region_count].sx = sx;
    manager->regions[manager->region_count].sy = sy;
    manager->regions[manager->region_count].w = w;
    manager->regions[manager->region_count].h = h;
    manager->regions[manager->region_count].angle = angle;
    manager->regions[manager->region_count].bitmap = bitmap;
    manager->region_count++;

    return bitmap;
}
