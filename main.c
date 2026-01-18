#include <raylib.h>
#include <unistd.h>

const int FPS = 25;

int main() {
    InitWindow(800, 450, "Hello world");
    Texture2D vent_00 = LoadTexture("assets/sprites/vent_00.png");
    Texture2D vent_01 = LoadTexture("assets/sprites/vent_01.png");

    int frame_n = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawTexture(frame_n % 25 < 12 ? vent_00 : vent_01, 16, 16, WHITE);
            DrawText("Hello, world!", 100, 100, 20, BLACK);
        EndDrawing();

        usleep(1000000 / FPS);
        frame_n++;
    }

    CloseWindow();
    return 0;
}
