#include "raylib.h"

#include "Config.h"
#include "Renderer.h"
#include "Simulation.h"

// Keyboard and mouse controls
static void handleInput(Simulation& sim, const Renderer& renderer, bool& paused, int& speed) {
    Config& cfg = sim.config();

    // IsKeyPressed only fires once per press, so holding a key doesn't repeat it every frame
    if (IsKeyPressed(KEY_SPACE)) {
        paused = !paused;
    }

    // Speed doubles or halves: 1x, 2x, 4x ... up to maxSpeed
    if (IsKeyPressed(KEY_UP) && speed < cfg.maxSpeed) {
        speed *= 2;
    }
    if (IsKeyPressed(KEY_DOWN) && speed > 1) {
        speed /= 2;
    }

    // Reset keeps the placed food so the same layout can be rerun
    if (IsKeyPressed(KEY_R)) {
        sim.reset();
    }
    if (IsKeyPressed(KEY_C)) {
        sim.clearFood();
    }

    // Toxicity for the next food placed, kept between 0 and 1
    if (IsKeyPressed(KEY_LEFT_BRACKET)) {
        cfg.foodToxicity -= 0.1f;
        if (cfg.foodToxicity < 0.0f) cfg.foodToxicity = 0.0f;
    }
    if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
        cfg.foodToxicity += 0.1f;
        if (cfg.foodToxicity > 1.0f) cfg.foodToxicity = 1.0f;
    }

    // Amount for the next food placed, at least 10
    if (IsKeyPressed(KEY_MINUS) && cfg.foodAmount > 10) {
        cfg.foodAmount -= 10;
    }
    if (IsKeyPressed(KEY_EQUAL)) {
        cfg.foodAmount += 10;
    }

    // Click on the grid to place food (doc 14.1: food can be added while running)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int x, y;
        if (renderer.screenToCell(GetMousePosition(), x, y)) {
            sim.addFood(x, y);
        }
    }
}

int main(){
    Config cfg;

    InitWindow(cfg.gridWidth * cfg.cellSize + cfg.panelWidth, cfg.gridHeight * cfg.cellSize,
               "Ant Colony Poison Model");
    SetTargetFPS(cfg.targetFps);

    Simulation sim(cfg);
    Renderer renderer(cfg);
    bool paused = true;         // start paused so food can be placed first (doc 5.2)
    int speed = 1;              // multiplier on cfg.ticksPerSecond
    float tickTimer = 0.0f;     // time banked toward the next tick, like Ant::moveProgress

    while(!WindowShouldClose()){
        handleInput(sim, renderer, paused, speed);

        // Run the model at a set number of ticks per second, separate from the 60 FPS frame rate.
        // Bank each frame's time and run a tick for every full tick's worth that has built up
        if (!paused) {
            float frameTime = GetFrameTime();
            if (frameTime > 0.1f) frameTime = 0.1f;     // don't run a huge burst of ticks after a lag spike
            tickTimer += frameTime;

            float secondsPerTick = 1.0f / (cfg.ticksPerSecond * speed);
            while (tickTimer >= secondsPerTick) {
                sim.tick();
                tickTimer -= secondsPerTick;
            }
        }

        BeginDrawing();
        renderer.draw(sim, paused, speed);
        EndDrawing();
    }

    CloseWindow();
}
