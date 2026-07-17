#include "entity.h"
#include "log.h"
#include <math.h>
#define ANGLE_90_DEG (64)

static BITMAP* rotation_scratch = NULL;

Entity* create_entity(int ex, int ey, int ew, int eh){
    Entity* entity = (Entity*)malloc(sizeof(Entity));
    if(!entity){
        log_debug("Failed to allocate memory for Entity.\n");
        return NULL;
    }

    if(ew <= 0 || eh <= 0){
        log_debug("Invalid size for entity.\n");
        free(entity);
        return NULL;
    }

    entity->x = ex;
    entity->y = ey;
    entity->w = ew;
    entity->h = eh;
    entity->scale = 1.0;
    entity->sheet = NULL;
    entity->sx = 0;
    entity->sy = 0;
    entity->angle = 0;

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

void entity_renderer_shutdown(void) {
    if (!rotation_scratch)
        return;

    destroy_bitmap(rotation_scratch);
    rotation_scratch = NULL;
}

static BITMAP* get_rotation_scratch(int w, int h) {
    if (rotation_scratch && rotation_scratch->w == w && rotation_scratch->h == h)
        return rotation_scratch;

    entity_renderer_shutdown();
    rotation_scratch = create_bitmap(w, h);
    return rotation_scratch;
}

static void render_orthogonal_entity(BITMAP* scr, Entity* entity, int screen_x, int screen_y, int angle) {
    int scaled_w = (int)(entity->w * entity->scale);
    int scaled_h = (int)(entity->h * entity->scale);
    int src_w = (angle == 90 || angle == 270) ? entity->h : entity->w;
    int src_h = (angle == 90 || angle == 270) ? entity->w : entity->h;
    int mask = entity->sheet->vtable->mask_color;

    if (scaled_w <= 0 || scaled_h <= 0)
        return;

    for (int y = 0; y < scaled_h; ++y) {
        int dy = (y * entity->h) / scaled_h;

        for (int x = 0; x < scaled_w; ++x) {
            int dx = (x * entity->w) / scaled_w;
            int sx = 0;
            int sy = 0;

            switch (angle) {
            case 90:
                sx = dy;
                sy = src_h - 1 - dx;
                break;
            case 180:
                sx = src_w - 1 - dx;
                sy = src_h - 1 - dy;
                break;
            case 270:
                sx = src_w - 1 - dy;
                sy = dx;
                break;
            default:
                sx = dx;
                sy = dy;
                break;
            }

            int color = getpixel(entity->sheet, entity->sx + sx, entity->sy + sy);
            if (color != mask)
                putpixel(scr, screen_x + x, screen_y + y, color);
        }
    }
}

void render_entity(BITMAP* scr, Entity* entity, const Box* vp) {
    if (!entity || !entity->sheet) return;

    int screen_x = entity->x - (vp ? vp->Left : 0);
    int screen_y = entity->y - (vp ? vp->Top : 0);
    int scaled_w = (int)(entity->w * entity->scale);
    int scaled_h = (int)(entity->h * entity->scale);
    int angle = entity->angle % 360;
    if (angle < 0)
        angle += 360;

    if (angle == 0) {
        masked_stretch_blit(entity->sheet, scr,
                                entity->sx, entity->sy,
                                entity->w, entity->h,
                                screen_x, screen_y,
                                scaled_w, scaled_h);
    } else if (angle == 90 || angle == 180 || angle == 270) {
        render_orthogonal_entity(scr, entity, screen_x, screen_y, angle);
    } else {
        BITMAP* temp_sub = get_rotation_scratch(entity->w, entity->h);
        if (!temp_sub) return;

        clear_to_color(temp_sub, entity->sheet->vtable->mask_color);
        blit(entity->sheet, temp_sub, entity->sx, entity->sy, 0, 0, entity->w, entity->h);

        int allegro_angle = (angle * 256) / 360;
        fixed render_angle = itofix(allegro_angle);

        fixed scale_fixed = ftofix(entity->scale);

        int target_center_x = screen_x + (scaled_w / 2);
        int target_center_y = screen_y + (scaled_h / 2);

        int pivot_x = temp_sub->w / 2;
        int pivot_y = temp_sub->h / 2;

        pivot_scaled_sprite(scr, temp_sub,
                            target_center_x, target_center_y,
                            pivot_x, pivot_y,
                            render_angle, scale_fixed);
    }
}
