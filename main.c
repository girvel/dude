#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nob.h"
#include "src/field.h"
#include "src/systems/display_grid.h"
#include "src/modern.h"


Field field;
// SoA optimization for (theoretically) CPU caching

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
    display_grid_init();

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
            ClearBackground(WHITE);
            display_grid(&field, frame_n);

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

    display_grid_deinit();
    CloseWindow();
    return 0;
}
