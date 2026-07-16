#include "entity.h"
#include "log.h"
#include <math.h>
#define ANGLE_90_DEG (64)

Entity* create_entity(int ex, int ey, int ew, int eh){
    Entity* entity = (Entity*)malloc(sizeof(Entity));
    if(!entity){
        log_debug("Failed to allocate memory for Entity.\n");
        return NULL;
    }

    if(ew <= 0 || eh <= 0){
        log_debug("Invalid size for entity.\n");
        return NULL;
    }

    entity->x = ex;
    entity->y = ey;
    entity->w = ew;
    entity->h = eh;
    entity->scale = 1.0;
    entity->sheet = NULL;

    return entity;
}

void add_sprite_to_entity(TextureManager* manager, Entity* entity, const char* path, int sx, int sy) {
    if (!manager || !entity) {
        return;
    }

    if (sx < 0 || sy < 0) {
        log_debug("Invalid offset for entity graphics.\n");
        return;
    }

    BITMAP* sheet = load_texture(manager, path);
    if (!sheet) {
        log_debug("Failed to load sprite from path: %s\n", path);
        return;
    }

    entity->sheet = sheet;
    entity->sx = sx;
    entity->sy = sy;
    entity->angle = 0;
}

void entity_rotate_right(Entity* entity) {
    if (!entity || !entity->sheet) {
        return;
    }

    entity->angle = (entity->angle + 90) % 360;

    int old_w = entity->w;
    entity->w = entity->h;
    entity->h = old_w;
}

void destroy_entity(Entity* entity) {
    if (!entity) {
        return;
    }

    free(entity);
}

void render_entity(BITMAP* scr, Entity* entity, const Box* vp) {
    if (!entity || !entity->sheet) return;

    int screen_x = entity->x - (vp ? vp->Left : 0);
    int screen_y = entity->y - (vp ? vp->Top : 0);
    int scaled_w = (int)(entity->w * entity->scale);
    int scaled_h = (int)(entity->h * entity->scale);
    if (entity->angle == 0) {
        masked_stretch_blit(entity->sheet, scr,
                                entity->sx, entity->sy,
                                entity->w, entity->h,
                                screen_x, screen_y,
                                scaled_w, scaled_h);
    } else {
        BITMAP* temp_sub = create_sub_bitmap(entity->sheet, entity->sx, entity->sy,
                                             entity->h, entity->w); // Swapped for rotation bounds
        if (!temp_sub) return;

        int allegro_angle = (entity->angle * 256) / 360;
        fixed angle = itofix(allegro_angle);

        fixed scale_fixed = ftofix(entity->scale);

        int target_center_x = screen_x + (scaled_w / 2);
        int target_center_y = screen_y + (scaled_h / 2);

        int pivot_x = temp_sub->w / 2;
        int pivot_y = temp_sub->h / 2;

        pivot_scaled_sprite(scr, temp_sub,
                            target_center_x, target_center_y,
                            pivot_x, pivot_y,
                            angle, scale_fixed);

        destroy_bitmap(temp_sub);
    }
}

static int get_block_def(int id, int* left, int* top){
    switch(id){
        case BLOCK_BARELAND:
        *left = 0;
        *top = 0;

        return 0;
        case BLOCK_STONE:
        *left = 16;
        *top = 0;

        return 0;
        case BLOCK_GRASS:
        *left = 32;
        *top = 0;

        return 0;
        case BLOCK_WATER:
        *left = 48;
        *top = 0;

        return 0;
        case BLOCK_SAND:
        *left = 0;
        *top = 16;

        return 0;
        case BLOCK_ORE_COPPER:
        *left = 16;
        *top = 16;

        return 0;
        case BLOCK_ORE_IRON:
        *left = 32;
        *top = 16;

        return 0;
        case BLOCK_ORE_COAL:
        *left = 48;
        *top = 16;

        return 0;
    }

    return ERROR_NOT_FOUND;
}

static int get_overlay_def(int id, int* left, int* top){
    return ERROR_NOT_FOUND;
}

int get_tile_def(int id, int* left, int* top){
    switch(id & 0xF000){
        case TILE_BLOCK:
            return get_block_def(id, left, top);
        case TILE_OVERLAY:
            return get_overlay_def(id, left, top);
    }

    return ERROR_INVALID;
}
