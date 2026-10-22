#include "Renderer.h"

Renderer::Renderer(const Config& cfg) {
    cellSize = cfg.cellSize;

    // Size of the grid area in pixels
    worldWidth = cfg.gridWidth * cfg.cellSize;
    worldHeight = cfg.gridHeight * cfg.cellSize;

    // Stats panel sits right of the grid
    panelX = worldWidth;
    panelWidth = cfg.panelWidth;
}

void Renderer::draw(const Simulation& sim, bool paused, int speed) const {
    // Dark ground color
    ClearBackground(Color{30, 26, 22, 255});

    // Draw back to front, so ants end up on top of trails and food
    drawPheromones(sim.getGrid(), sim.config().pheromoneMax);
    drawFood(sim.getFood(), sim.config().foodAmount);
    drawNest(sim.getNest());
    drawAnts(sim.getAnts());
    drawHud(sim, paused, speed);
}

bool Renderer::screenToCell(Vector2 screen, int& cellX, int& cellY) const {
    // Clicked outside the grid, like on the stats panel
    if (screen.x < 0 || screen.y < 0 || screen.x >= worldWidth || screen.y >= worldHeight) {
        return false;
    }

    // Each cell is cellSize pixels wide, so dividing gives the cell number
    cellX = (int)(screen.x / cellSize);
    cellY = (int)(screen.y / cellSize);
    return true;
}

void Renderer::drawPheromones(const Grid& grid, float maxPheromone) const {
    Color trailColor = Color{80, 160, 255, 255};    // Adjust this later possibly???

    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            float strength = grid.pheromoneAt(x, y);

            // Most cells have no pheromone, skip them
            if (strength <= 0.0f) continue;

            // Stronger trails are drawn more solid
            float opacity = 0.2f + 0.8f * (strength / maxPheromone);
            DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, Fade(trailColor, opacity));
        }
    }
}

void Renderer::drawFood(const std::vector<FoodSource>& food, int maxAmount) const {
    for (const FoodSource& source : food) {
        // Center of the food's cell in pixels
        int centerX = source.x * cellSize + cellSize / 2;
        int centerY = source.y * cellSize + cellSize / 2;

        // Empty piles get a faint outline, so you can see trails that lead to nothing
        if (source.isDepleted()) {
            DrawCircleLines(centerX, centerY, cellSize, Fade(GRAY, 0.5f));
            continue;
        }

        // Pile shrinks as ants take from it. Capped at 1 in case it was placed with a bigger amount
        float fullness = (float)source.amount / maxAmount;
        if (fullness > 1.0f) fullness = 1.0f;
        float radius = 3.0f + fullness * cellSize * 1.5f;     // 3 to 12 pixels

        // Green is harmless, purple is fully toxic
        Color color = ColorLerp(GREEN, PURPLE, source.toxicity);
        DrawCircle(centerX, centerY, radius, color);
    }
}

void Renderer::drawNest(const Nest& nest) const {
    // Center of the nest's cell in pixels
    int centerX = nest.x() * cellSize + cellSize / 2;
    int centerY = nest.y() * cellSize + cellSize / 2;
    float radius = cellSize * 2.5f;

    // Brown when healthy, turns red as poison builds up
    float healthFraction = nest.getHealth() / nest.getMaxHealth();
    Color color = ColorLerp(RED, BROWN, healthFraction);

    DrawCircle(centerX, centerY, radius, color);
    DrawCircleLines(centerX, centerY, radius, DARKBROWN);
}

void Renderer::drawAnts(const std::vector<Ant>& ants) const {
    for (const Ant& ant : ants) {
        // Light gray while searching, yellow while carrying food home
        Color color = LIGHTGRAY;
        if (ant.getState() == AntState::Returning) {
            color = YELLOW;
        }

        // Turns red as poison lowers its health (doc 2.3)
        color = ColorLerp(RED, color, ant.getHealthFraction());

        DrawRectangle(ant.x() * cellSize, ant.y() * cellSize, cellSize, cellSize, color);
    }
}

void Renderer::drawHud(const Simulation& sim, bool paused, int speed) const {
    const Nest& nest = sim.getNest();
    const Config& cfg = sim.config();

    // Panel background, right of the grid
    DrawRectangle(panelX, 0, panelWidth, worldHeight, Color{18, 18, 22, 255});

    int x = panelX + 15;            // left margin inside the panel
    int width = panelWidth - 30;    // usable width, with a margin on both sides
    int y = 15;                     // moves down the panel as each line is drawn

    DrawText("Ant Colony Poison Model", x, y, 20, RAYWHITE);
    y += 35;

    // Run stats. TextFormat works like printf and returns the finished text
    DrawText(TextFormat("Tick: %d", sim.getTick()), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawText(TextFormat("Ants alive: %d / %d", nest.getPopulation(), cfg.antCount), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawText(TextFormat("Food delivered: %d", nest.getFoodDelivered()), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawText(TextFormat("Poison delivered: %.1f", nest.getTotalPoisonDelivered()), x, y, 16, LIGHTGRAY);
    y += 30;

    // Nest health bar, green when healthy and red when nearly dead
    float healthFraction = nest.getHealth() / nest.getMaxHealth();
    DrawText(TextFormat("Nest health: %.0f / %.0f", nest.getHealth(), nest.getMaxHealth()), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawRectangle(x, y, width, 14, DARKGRAY);
    DrawRectangle(x, y, (int)(width * healthFraction), 14, ColorLerp(RED, GREEN, healthFraction));
    y += 30;

    // Population and nest health over time. Grows with the window so a tall panel isn't mostly empty
    int graphHeight = worldHeight / 4;
    Rectangle graphArea = {(float)x, (float)y, (float)width, (float)graphHeight};
    drawGraph(sim.getHistory(), cfg.antCount, nest.getMaxHealth(), graphArea);
    y += graphHeight + 30;      // graph plus its legend

    // Settings used for the next food source placed
    DrawText(TextFormat("New food toxicity: %.1f", cfg.foodToxicity), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawText(TextFormat("New food amount: %d", cfg.foodAmount), x, y, 16, LIGHTGRAY);
    y += 22;
    DrawText(TextFormat("Speed: %dx", speed), x, y, 16, LIGHTGRAY);
    y += 30;

    // Run status
    if (sim.getOutcome() == Outcome::NestPoisoned) {
        DrawText("COLONY POISONED", x, y, 20, RED);
    }
    else if (sim.getOutcome() == Outcome::AllAntsDead) {
        DrawText("ALL ANTS DEAD", x, y, 20, ORANGE);
    }
    else if (paused) {
        DrawText("PAUSED", x, y, 20, YELLOW);
    }

    // Controls list at the bottom of the panel (5 lines, 18 pixels apart)
    y = worldHeight - 5 * 18 - 10;
    DrawText("Space  pause / resume", x, y, 14, GRAY);
    y += 18;
    DrawText("Up / Down  speed", x, y, 14, GRAY);
    y += 18;
    DrawText("R  reset      C  clear food", x, y, 14, GRAY);
    y += 18;
    DrawText("[ ]  toxicity      - =  amount", x, y, 14, GRAY);
    y += 18;
    DrawText("Left click  place food", x, y, 14, GRAY);
}

void Renderer::drawGraph(const std::vector<StatSample>& history, int startPopulation,
                         float nestMaxHealth, Rectangle area) const {
    // Background and border
    DrawRectangleRec(area, Color{10, 10, 12, 255});
    DrawRectangleLinesEx(area, 1, DARKGRAY);

    // Legend under the graph
    int legendY = (int)(area.y + area.height + 6);
    DrawText("Ants alive", (int)area.x, legendY, 14, SKYBLUE);
    DrawText("Nest health", (int)area.x + 100, legendY, 14, ORANGE);

    // Need at least two points to draw a line
    int n = (int)history.size();
    if (n < 2) return;

    // Long runs have more samples than the graph is pixels wide, so skip some to keep drawing fast
    int step = 1 + n / (int)area.width;

    for (int i = step; i < n; i += step) {
        int prev = i - step;

        // Spread the samples evenly across the graph's width
        float x1 = area.x + area.width * prev / (n - 1);
        float x2 = area.x + area.width * i / (n - 1);

        // Both lines are a 0 to 1 fraction so they share the same scale. 1 is the top of the graph
        float pop1 = (float)history[prev].population / startPopulation;
        float pop2 = (float)history[i].population / startPopulation;
        DrawLineV(Vector2{x1, area.y + area.height * (1 - pop1)},
                  Vector2{x2, area.y + area.height * (1 - pop2)}, SKYBLUE);

        float health1 = history[prev].nestHealth / nestMaxHealth;
        float health2 = history[i].nestHealth / nestMaxHealth;
        DrawLineV(Vector2{x1, area.y + area.height * (1 - health1)},
                  Vector2{x2, area.y + area.height * (1 - health2)}, ORANGE);
    }
}
