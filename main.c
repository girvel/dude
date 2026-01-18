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

    Texture2D vent_00 = load_sprite("assets/sprites/vent_00.png");
    Texture2D vent_01 = load_sprite("assets/sprites/vent_01.png");

    Texture2D tank[] = {
        load_sprite("assets/sprites/tank_8_00.png"),
        load_sprite("assets/sprites/tank_8_01.png"),
        load_sprite("assets/sprites/tank_8_02.png"),
        load_sprite("assets/sprites/tank_8_03.png"),
    };

    entity_types[to_index(3, 3)] = SpriteId_Vent;
    entity_types[to_index(4, 5)] = SpriteId_Vent;
    entity_types[to_index(3, 5)] = SpriteId_Tank;

    oil[to_index(1, 1)] = 5;

    int frame_n = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
            // DRAW SYSTEM //
            ClearBackground(WHITE);
            for (size_t i = 0; i < FIELD_LEN; i++) {
                Texture2D texture;
                switch (entity_types[i]) {
                case SpriteId_None: continue;
                case SpriteId_Vent:
                    texture = oil[i] > 0 && frame_n % fps < (fps / 2)
                        ? vent_00
                        : vent_01;
                    break;
                case SpriteId_Tank:
                    texture = tank[(frame_n / (fps / 4)) % 4];
                    break;
                default:
                    NOB_UNREACHABLE("Unknown sprite");
                    break;
                }

                int x = i % FIELD_W;
                int y = i / FIELD_H;
                DrawTexture(texture, x * total_sprite_size, y * total_sprite_size, WHITE);
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

    UnloadTexture(vent_00);
    UnloadTexture(vent_01);
    CloseWindow();
    return 0;
}
