#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include "nob.h"


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
} EntityType;

EntityType entity_types[FIELD_LEN];

size_t to_index(size_t x, size_t y) {
    return y * FIELD_W + x;
}

int main() {
    InitWindow(FIELD_H * total_sprite_size, FIELD_W * total_sprite_size, "Hello world");

    // INITIALIZATION //
    memset(entity_types, 0, FIELD_LEN * sizeof(EntityType));
    Texture2D vent_00 = load_sprite("assets/sprites/vent_00.png");
    Texture2D vent_01 = load_sprite("assets/sprites/vent_01.png");
    entity_types[to_index(0, 0)] = SpriteId_Vent;
    entity_types[to_index(1, 1)] = SpriteId_Vent;

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
                    texture = frame_n % 25 < 12 ? vent_00 : vent_01;
                    break;
                default:
                    NOB_UNREACHABLE("Unknown sprite");
                    break;
                }

                int x = i % FIELD_W;
                int y = i / FIELD_H;
                DrawTexture(texture, x * total_sprite_size, y * total_sprite_size, WHITE);
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
