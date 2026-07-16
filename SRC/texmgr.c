#include "texmgr.h"
#include <stdio.h>

TextureManager* create_texture_manager() {
    TextureManager* manager = (TextureManager*)malloc(sizeof(TextureManager));
    if (!manager) {
        return NULL;
    }
    manager->textures = NULL;
    manager->texture_count = 0;
    return manager;
}

void destroy_texture_manager(TextureManager* manager){
    if (manager) {
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
