#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

#define FIELD_H 15
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
    EntityType_PipeUp,
    EntityType_Pump,
    EntityType_Block0,
    EntityType_Block1,
} EntityType;

EntityType entity_types[FIELD_LEN];
int oil[FIELD_LEN];
int oil_limit[FIELD_LEN];

size_t to_index(size_t x, size_t y) {
    return y * FIELD_W + x;
}

void from_index(size_t i, int *x, int *y) {
    *x = i % FIELD_W;
    *y = i / FIELD_W;
}

void put_vent(int x, int y) {
    size_t i = to_index(x, y);
    entity_types[i] = EntityType_Vent;
    oil[i] = 2;
    oil_limit[i] = 2;
}

void put_tank(int x, int y) {
    size_t i = to_index(x, y);
    entity_types[i] = EntityType_Tank;
    oil[i] = 8;
    oil_limit[i] = 8;
}

void put_pipe_up(int x, int y) {
    size_t i = to_index(x, y);
    entity_types[i] = EntityType_PipeUp;
    oil[i] = 0;
    oil_limit[i] = 1;
}

void put_pump(int x, int y) {
    size_t i = to_index(x, y);
    entity_types[i] = EntityType_Pump;
    oil[i] = 0;
    oil_limit[i] = 0;
}

int main() {
    srand(time(NULL));
    InitWindow(FIELD_W * total_sprite_size, FIELD_H * total_sprite_size, "Hello world");

    // INITIALIZATION //
    memset(entity_types, 0, FIELD_LEN * sizeof(entity_types[0]));
    memset(oil, 0, FIELD_LEN * sizeof(oil[0]));
    memset(oil_limit, 0, FIELD_LEN * sizeof(oil_limit[0]));

    Textures none = load_animation("none", 1);
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
    Textures pipe_up = load_animation("pipe_up", 5);
    Textures pipe_up_stopped = load_animation("pipe_up", 1);
    Textures pump = load_animation("pump", 1);
    Textures block_0 = load_animation("block_0", 1);
    Textures block_1 = load_animation("block_1", 1);

    for (size_t _ = rand() % 5 + 4; _ > 0; _--) {
        entity_types[rand() % FIELD_LEN] = EntityType_Block0;
    }

    for (size_t _ = rand() % 5 + 4; _ > 0; _--) {
        entity_types[rand() % FIELD_LEN] = EntityType_Block1;
    }

    put_vent(3, 3);
    put_vent(4, 5);
    put_tank(3, 5);
    put_tank(2, 5);
    put_pipe_up(3, 6);
    put_pipe_up(3, 7);
    put_pipe_up(3, 8);
    put_pipe_up(3, 9);
    put_pipe_up(3, 10);
    put_pump(3, 11);

    put_tank(9, 2);
    put_tank(10, 2);
    put_tank(11, 2);
    put_vent(12, 2);
    put_pipe_up(9, 3);
    put_pipe_up(10, 3);
    put_pipe_up(11, 3);
    put_pump(9, 4);
    put_pump(10, 4);
    put_pump(11, 4);

    entity_types[to_index(18, 6)] = EntityType_Block1;

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
                case EntityType_PipeUp:
                    animation = oil[i] > 0 ? pipe_up : pipe_up_stopped;
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

            // OIL SYSTEM //
            int oil_next[FIELD_LEN];
            memcpy(oil_next, oil, FIELD_LEN * sizeof(int));

            for (size_t i = 0; i < FIELD_LEN; i++) {
                int x, y;
                from_index(i, &x, &y);

                switch(entity_types[i]) {
                case EntityType_Vent:
                    if (frame_n % fps == 0) {
                        oil_next[i] = MAX2(oil[i] - 1, 0);
                    }
                    break;

                case EntityType_Tank:
                    if (frame_n % 5 == 0 && oil[i] > 0) {
                        size_t j = to_index(x + 1, y);
                        if (x + 1 < FIELD_W
                            && (entity_types[j] == EntityType_Vent
                                || entity_types[j] == EntityType_Tank)
                            && oil[j] <= 1)
                        {
                            oil_next[j]++;
                            oil_next[i]--;
                        }
                    }

                case EntityType_PipeUp:
                    if (frame_n % fps == 0 && oil[i] > 0) {
                        size_t j = to_index(x, y - 1);
                        if (y - 1 >= 0
                            && entity_types[j] != EntityType_None
                            && oil[j] + 1 <= oil_limit[j])
                        {
                            oil_next[i]--;
                            oil_next[j]++;
                        }
                    }
                    break;

                case EntityType_Pump: {
                    size_t j = to_index(x, y - 1);
                    if (frame_n % (3 * fps) == 0 && oil[j] == 0) {
                        oil_next[i]--;
                        oil_next[j]++;
                    }
                    } break;

                default:
                    break;
                }
            }

            memcpy(oil, oil_next, FIELD_LEN * sizeof(int));
        EndDrawing();

        usleep(1000000 / fps);
        frame_n++;
    }

    CloseWindow();
    return 0;
}
