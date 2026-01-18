#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include "nob.h"


#define MAX2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a > _b ? _a : _b; \
})

#define MIN2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a < _b ? _a : _b; \
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
    EntityType_None = 0,
    EntityType_Vent,
    EntityType_Tank,
} EntityType;

EntityType entity_types[FIELD_LEN];
int oil[FIELD_LEN];

size_t to_index(size_t x, size_t y) {
    return y * FIELD_W + x;
}

void from_index(size_t i, int *x, int *y) {
    *x = i % FIELD_W;
    *y = i / FIELD_W;
}

int main() {
    InitWindow(FIELD_H * total_sprite_size, FIELD_W * total_sprite_size, "Hello world");

    // INITIALIZATION //
    memset(entity_types, 0, FIELD_LEN * sizeof(entity_types[0]));
    memset(oil, 0, FIELD_LEN * sizeof(oil[0]));

    Textures vent = load_animation("vent", 2);
    Textures vent_stopped = load_animation("vent", 1);
    Textures tank[] = {
        load_animation("tank_0", 4),
        load_animation("tank_1", 4),
        load_animation("tank_2", 4),
        load_animation("tank_3", 4),
        load_animation("tank_4", 4),
        load_animation("tank_5", 4),
        load_animation("tank_6", 4),
        load_animation("tank_7", 4),
        load_animation("tank_8", 4),
    };
    Textures none = load_animation("none", 1);

    entity_types[to_index(3, 3)] = EntityType_Vent;
    entity_types[to_index(4, 5)] = EntityType_Vent;
    entity_types[to_index(3, 5)] = EntityType_Tank;

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
                case EntityType_None:
                    animation = none;
                    break;
                case EntityType_Vent:
                    animation = oil[i] > 0 ? vent : vent_stopped;
                    break;
                case EntityType_Tank:
                    animation = tank[MIN2(8, oil[i])];
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
            for (size_t i = 0; i < FIELD_LEN; i++) {  // TODO: oil_next array
                if (entity_types[i] == EntityType_Vent && frame_n % fps == 0) {
                    oil[i] = MAX2(oil[i] - 1, 0);
                }

                if (entity_types[i] == EntityType_Tank && frame_n % 5 == 0) {
                    if (oil[i] > 0) {
                        int x, y;
                        from_index(i, &x, &y);

                        size_t j = to_index(x + 1, y);
                        if (x + 1 < FIELD_W
                            && entity_types[j] == EntityType_Vent
                            && oil[j] <= 1)
                        {
                            oil[j]++;
                            oil[i]--;
                        }
                    }
                }
            }
        EndDrawing();

        usleep(1000000 / fps);
        frame_n++;
    }

    CloseWindow();
    return 0;
}
