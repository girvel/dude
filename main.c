#include <raylib.h>
#include <unistd.h>

const int FPS = 25;
const int SCALE = 4;

Texture2D LoadSprite(const char *path) {
    Image img = LoadImage(path);
    ImageResizeNN(&img, img.width * SCALE, img.height * SCALE);
    Texture2D result = LoadTextureFromImage(img);
    UnloadImage(img);
    return result;
}

int main() {
    InitWindow(800, 450, "Hello world");
    Texture2D vent_00 = LoadSprite("assets/sprites/vent_00.png");
    Texture2D vent_01 = LoadSprite("assets/sprites/vent_01.png");

    int frame_n = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawTexture(frame_n % 25 < 12 ? vent_00 : vent_01, 64, 64, WHITE);
            DrawText("Hello, world!", 200, 100, 20, BLACK);
        EndDrawing();

        usleep(1000000 / FPS);
        frame_n++;
    }

    UnloadTexture(vent_00);
    UnloadTexture(vent_01);
    CloseWindow();
    return 0;
}
