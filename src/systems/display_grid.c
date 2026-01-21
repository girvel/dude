#include "display_grid.h"

#include <assert.h>
#include <stddef.h>
#include <raylib.h>
#include <stdlib.h>
#include "../../nob.h"
#include "../modern.h"

static Texture2D load_sprite(const char *path) {
    Image img = LoadImage(path);
    ImageResizeNN(&img, img.width * scale, img.height * scale);
    Texture2D result = LoadTextureFromImage(img);
    UnloadImage(img);
    return result;
}

typedef struct {
    Texture2D *items;
    size_t count;
} Textures;

static Textures load_animation(const char *id, size_t count) {
    Textures result = {
        .items = malloc(sizeof(Texture2D) * count),
        .count = count,
    };

    assert(result.items != NULL && "Get more RAM lol");

    for (size_t i = 0; i < count; i++) {
        result.items[i] = load_sprite(nob_temp_sprintf("assets/sprites/%s_%02zu.png", id, i));
    }

    return result;
}

void free_animation(Textures *tex) {
    for (size_t i = 0; i < tex->count; i++) {
        UnloadTexture(tex->items[i]);
    }
    free(tex->items);
    tex->items = NULL;
    tex->count = 0;
}

static Textures none, vent, vent_stopped, pipe_up, pipe_up_stopped, pump, block_0, block_1;
static Texture2D gui;
static Textures tank[9];

void display_grid_init() {
    none = load_animation("none", 1);
    vent = load_animation("vent", 2);
    vent_stopped = load_animation("vent", 1);
    tank[0] = load_animation("tank_0", 4);
    tank[1] = load_animation("tank_1", 4);
    tank[2] = load_animation("tank_2", 4);
    tank[3] = load_animation("tank_3", 4);
    tank[4] = load_animation("tank_4", 4);
    tank[5] = load_animation("tank_5", 4);
    tank[6] = load_animation("tank_6", 4);
    tank[7] = load_animation("tank_7", 4);
    tank[8] = load_animation("tank_8", 4);
    pipe_up = load_animation("pipe_up", 5);
    pipe_up_stopped = load_animation("pipe_up", 1);
    pump = load_animation("pump", 1);
    block_0 = load_animation("block_0", 1);
    block_1 = load_animation("block_1", 1);
    gui = load_sprite("assets/sprites/gui.png");
}

void display_grid_deinit() {
    free_animation(&none);
    free_animation(&vent);
    free_animation(&vent_stopped);
    for (size_t i = 0; i < 9; i++) free_animation(&tank[i]);
    free_animation(&pipe_up);
    free_animation(&pipe_up_stopped);
    free_animation(&pump);
    free_animation(&block_0);
    free_animation(&block_1);
    UnloadTexture(gui);

}

void display_grid(Field *field, int frame_n) {
    for (size_t i = 0; i < FIELD_LEN; i++) {
        Textures animation;
        switch (field->type[i]) {
        case EntityType_None:
            animation = none;
            break;
        case EntityType_Vent:
            animation = field->oil[i] > 0 ? vent : vent_stopped;
            break;
        case EntityType_Tank:
            animation = tank[min_int(8, field->oil[i])];
            break;
        case EntityType_PipeUp:
            animation = field->oil[i] > 0 ? pipe_up : pipe_up_stopped;
            break;
        case EntityType_Pump:
            animation = pump;
            break;
        case EntityType_Block0:
            animation = block_0;
            break;
        case EntityType_Block1:
            animation = block_1;
            break;
        default:
            NOB_UNREACHABLE("Unknown sprite");
            break;
        }

        int x, y;
        from_index(i, &x, &y);
        Texture2D frame = animation.items[(frame_n / animation_frames_n) % animation.count];
        DrawTexture(frame, x * total_sprite_size, y * total_sprite_size, WHITE);
    }

    const int gui_x = FIELD_W * total_sprite_size - gui.width;
    const int padding = scale * 3;
    DrawTexture(gui, gui_x, 0, WHITE);
    DrawText(nob_temp_sprintf("Oil: %03d", 0), gui_x + padding, padding, 6 * scale, BLACK);
}
