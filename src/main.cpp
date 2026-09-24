#include "raylib.h"

int main(){
    // Raylib init
    InitWindow(800, 600, "test");
    SetTargetFPS(60);

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello", 20, 20, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
}