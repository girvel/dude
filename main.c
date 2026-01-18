#include <math.h>
#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include "nob.h"


#define MAX2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a > _b ? _a : _b; \
})

#define FIELD_H 20
#define FIELD_W 20
#define FIELD_LEN (FIELD_H * FIELD_W)

const int fps = 25;
const int animation_frames_n = 5;
const int scale = 4;
const int source_sprite_size = 16;
const int total_sprite_size = scale * source_sprite_size;

Texture2D load_sprite(const char *path) {
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

Textures load_animation(const char *id, size_t count) {
    Textures result = {
        .items = malloc(sizeof(Texture2D) * count),
        .count = count,
    };

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

typedef enum {
    SpriteId_None = 0,
    SpriteId_Vent,
    SpriteId_Tank,
} EntityType;

EntityType entity_types[FIELD_LEN];
int oil[FIELD_LEN];

size_t to_index(size_t x, size_t y) {
    return y * FIELD_W + x;
}

int main() {
    InitWindow(FIELD_H * total_sprite_size, FIELD_W * total_sprite_size, "Hello world");

    // INITIALIZATION //
    memset(entity_types, 0, FIELD_LEN * sizeof(entity_types[0]));
    memset(oil, 0, FIELD_LEN * sizeof(oil[0]));

    Textures vent = load_animation("vent", 2);
    Textures vent_stopped = load_animation("vent", 1);
    Textures tank = load_animation("tank_8", 4);
    Textures none = load_animation("none", 1);

    entity_types[to_index(3, 3)] = SpriteId_Vent;
    entity_types[to_index(4, 5)] = SpriteId_Vent;
    entity_types[to_index(3, 5)] = SpriteId_Tank;

    oil[to_index(4, 5)] = 5;
    oil[to_index(3, 5)] = 8;

    int frame_n = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
            // DRAW SYSTEM //
            ClearBackground(WHITE);
            for (size_t i = 0; i < FIELD_LEN; i++) {
                Textures animation;
                switch (entity_types[i]) {
                case SpriteId_None:
                    animation = none;
                    break;
                case SpriteId_Vent:
                    animation = oil[i] > 0 ? vent : vent_stopped;
                    break;
                case SpriteId_Tank:
                    animation = tank;
                    break;
                default:
                    NOB_UNREACHABLE("Unknown sprite");
                    break;
                }

                int x = i % FIELD_W;
                int y = i / FIELD_H;
                Texture2D frame = animation.items[(frame_n / animation_frames_n) % animation.count];
                DrawTexture(frame, x * total_sprite_size, y * total_sprite_size, WHITE);
            }

            // OIL SYSTEM //
            if (frame_n % fps == 0) {
                for (size_t i = 0; i < FIELD_LEN; i++) {
                    if (entity_types[i] == SpriteId_Vent) {
                        oil[i] = MAX2(oil[i] - 1, 0);
                    }
                }
            }
        EndDrawing();

        usleep(1000000 / fps);
        frame_n++;
    }

    free_animation(&tank);
    CloseWindow();
    return 0;
}
