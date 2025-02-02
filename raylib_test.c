#include <raylib.h>

int main(int argc, char const *argv[])
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Tictactoe client window");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Awawa", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
