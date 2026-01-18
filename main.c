#include <raylib.h>

int main() {
    InitWindow(800, 450, "Hello world");

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawText("Hello, world!", 100, 100, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
