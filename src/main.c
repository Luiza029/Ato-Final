#include "raylib.h"

int main(void)
{
    InitWindow(960, 540, "Ato Final");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Ato Final", 20, 20, 40, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}