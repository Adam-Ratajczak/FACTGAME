#ifndef ENTITY_H
#define ENTITY_H
#include "texmgr.h"
#include "structs.h"

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
void entity_renderer_shutdown(void);

void render_entity(BITMAP* scr, Entity* entity, const Box* vp);

#endif // ENTITY_H
