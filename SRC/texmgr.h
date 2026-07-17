
#include "alwrap.h"

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

typedef struct
{
    char* file_path;
    BITMAP* bitmap;
} Texture;

typedef struct
{
    char* file_path;
    int sx;
    int sy;
    int w;
    int h;
    int angle;
    BITMAP* bitmap;
} TextureRegion;

typedef struct {
    Texture* textures;
    int texture_count;
    TextureRegion* regions;
    int region_count;
} TextureManager;

TextureManager* create_texture_manager();
void destroy_texture_manager(TextureManager* manager);
BITMAP* load_texture(TextureManager* manager, const char* file_path);
BITMAP* load_texture_region(TextureManager* manager, const char* file_path, int sx, int sy, int w, int h, int angle);

#endif // TEXTURE_MANAGER_H
