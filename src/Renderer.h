#pragma once

#include <vector>

#include "raylib.h"
#include "Simulation.h"

// Draws the simulation with raylib. Reads from Simulation, never changes it
class Renderer{
    public:
        explicit Renderer(const Config& cfg);

        // Call between BeginDrawing() and EndDrawing()
        void draw(const Simulation& sim, bool paused, int speed) const;

        // Converts a mouse position to a grid cell. Returns false if it's outside the grid
        bool screenToCell(Vector2 screen, int& cellX, int& cellY) const;

    private:
        void drawPheromones(const Grid& grid, float maxPheromone) const;
        void drawFood(const std::vector<FoodSource>& food, int maxAmount) const;
        void drawNest(const Nest& nest) const;
        void drawAnts(const std::vector<Ant>& ants) const;
        void drawHud(const Simulation& sim, bool paused, int speed) const;
        void drawGraph(const std::vector<StatSample>& history, int startPopulation,
                       float nestMaxHealth, Rectangle area) const;

        int cellSize = 0;
        int worldWidth = 0;     // grid size in pixels
        int worldHeight = 0;
        int panelX = 0;         // left edge of the stats panel
        int panelWidth = 0;
};
