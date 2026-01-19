#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nob.h"


static inline int max_int(int a, int b) { return a > b ? a : b; }
static inline int min_int(int a, int b) { return a < b ? a : b; }

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

typedef enum {
    EntityType_None = 0,
    EntityType_Vent,
    EntityType_Tank,
    EntityType_PipeUp,
    EntityType_Pump,
    EntityType_Block0,
    EntityType_Block1,
} EntityType;

struct {
    EntityType type[FIELD_LEN];
    int oil[FIELD_LEN];
    int oil_limit[FIELD_LEN];
} field;
// SoA optimization for (theoretically) CPU caching

static inline size_t to_index(size_t x, size_t y) {
    assert(x < FIELD_W && y < FIELD_H && "Indexing out of game field bounds");
    return y * FIELD_W + x;
}

static inline void from_index(size_t i, int *x, int *y) {
    *x = i % FIELD_W;
    *y = i / FIELD_W;
}

void put_vent(int x, int y) {
    size_t i = to_index(x, y);
    field.type[i] = EntityType_Vent;
    field.oil[i] = 2;
    field.oil_limit[i] = 2;
}

void put_tank(int x, int y) {
    size_t i = to_index(x, y);
    field.type[i] = EntityType_Tank;
    field.oil[i] = 8;
    field.oil_limit[i] = 8;
}

void put_pipe_up(int x, int y) {
    size_t i = to_index(x, y);
    field.type[i] = EntityType_PipeUp;
    field.oil[i] = 0;
    field.oil_limit[i] = 1;
}

void put_pump(int x, int y) {
    size_t i = to_index(x, y);
    field.type[i] = EntityType_Pump;
    field.oil[i] = 0;
    field.oil_limit[i] = 0;
}

int main() {
    srand(time(NULL));
    InitWindow(FIELD_W * total_sprite_size, FIELD_H * total_sprite_size, "Hello world");
    SetTargetFPS(fps);

    // INITIALIZATION //
    memset(field.type, 0, FIELD_LEN * sizeof(field.type[0]));
    memset(field.oil, 0, FIELD_LEN * sizeof(field.oil[0]));
    memset(field.oil_limit, 0, FIELD_LEN * sizeof(field.oil_limit[0]));

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

    for (int _ = rand() % 5 + 4; _ > 0; _--) {
        field.type[rand() % FIELD_LEN] = EntityType_Block0;
    }

    for (int _ = rand() % 5 + 4; _ > 0; _--) {
        field.type[rand() % FIELD_LEN] = EntityType_Block1;
    }

    const int padding = 2;
    for (int _ = rand() % 2 + 2; _ > 0; _--) {
        int pipe_lengths[4];
        int tanks_n = rand() % 4;
        int pipe_max_length = 0;
        for (int i = 0; i < tanks_n; i++) {
            pipe_lengths[i] = rand() % 5 + 1;
            pipe_max_length = max_int(pipe_lengths[i], pipe_max_length);
        }

        int x = padding + rand() % (FIELD_W - 2 * padding - tanks_n - 1);
        int y = padding + rand() % (FIELD_H - 2 * padding - pipe_max_length - 2);

        for (int i = 0; i < tanks_n; i++) {
            put_tank(x + i, y);
            for (int j = 0; j < pipe_lengths[i]; j++) {
                put_pipe_up(x + i, y + 1 + j);
            }
            put_pump(x + i, y + 1 + pipe_lengths[i]);
        }
        put_vent(x + tanks_n, y);
    }

    int frame_n = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
            // DRAW SYSTEM //
            ClearBackground(WHITE);
            for (size_t i = 0; i < FIELD_LEN; i++) {
                Textures animation;
                switch (field.type[i]) {
                case EntityType_None:
                    animation = none;
                    break;
                case EntityType_Vent:
                    animation = field.oil[i] > 0 ? vent : vent_stopped;
                    break;
                case EntityType_Tank:
                    animation = tank[min_int(8, field.oil[i])];
                    break;
                case EntityType_PipeUp:
                    animation = field.oil[i] > 0 ? pipe_up : pipe_up_stopped;
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
            memcpy(oil_next, field.oil, FIELD_LEN * sizeof(int));

            for (size_t i = 0; i < FIELD_LEN; i++) {
                int x, y;
                from_index(i, &x, &y);

                switch(field.type[i]) {
                case EntityType_Vent:
                    if (frame_n % fps == 0) {
                        oil_next[i] = max_int(field.oil[i] - 1, 0);
                    }
                    break;

                case EntityType_Tank:
                    if (frame_n % 5 == 0 && field.oil[i] > 0) {
                        size_t j = to_index(x + 1, y);
                        if (x + 1 < FIELD_W
                            && (field.type[j] == EntityType_Vent
                                || field.type[j] == EntityType_Tank)
                            && field.oil[j] <= 1)
                        {
                            oil_next[j]++;
                            oil_next[i]--;
                        }
                    }
                    break;

                case EntityType_PipeUp:
                    if (frame_n % fps == 0 && field.oil[i] > 0) {
                        size_t j = to_index(x, y - 1);
                        if (y - 1 >= 0
                            && field.type[j] != EntityType_None
                            && field.oil[j] + 1 <= field.oil_limit[j])
                        {
                            oil_next[i]--;
                            oil_next[j]++;
                        }
                    }
                    break;

                case EntityType_Pump: {
                    size_t j = to_index(x, y - 1);
                    if (frame_n % (3 * fps) == 0 && field.oil[j] == 0) {
                        oil_next[j]++;
                    }
                    } break;

                default:
                    break;
                }
            }

            memcpy(field.oil, oil_next, FIELD_LEN * sizeof(int));
        EndDrawing();
        frame_n++;
    }

    free_animation(&none);
    free_animation(&vent);
    free_animation(&vent_stopped);
    for (size_t i = 0; i < 9; i++) free_animation(&tank[i]);
    free_animation(&pipe_up);
    free_animation(&pipe_up_stopped);
    free_animation(&pump);
    free_animation(&block_0);
    free_animation(&block_1);

    CloseWindow();
    return 0;
}
