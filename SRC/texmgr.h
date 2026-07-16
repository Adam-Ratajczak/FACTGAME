
#include "alwrap.h"

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

typedef struct
{
    char* file_path;
    BITMAP* bitmap;
} Texture;

typedef struct {
    Texture* textures;
    int texture_count;
} TextureManager;

TextureManager* create_texture_manager();
void destroy_texture_manager(TextureManager* manager);
BITMAP* load_texture(TextureManager* manager, const char* file_path);

#endif // TEXTURE_MANAGER_H