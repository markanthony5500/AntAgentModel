#pragma once

#include <random>
#include <vector>

#include "Ant.h"
#include "Config.h"
#include "FoodSource.h"
#include "Grid.h"
#include "Nest.h"

// Why a run ended
enum class Outcome { Running, NestPoisoned, AllAntsDead };

// One point on the stats graph
struct StatSample {
    int tick;
    int population;
    float nestHealth;
};

// Main class to simulate the model one tick at a time
class Simulation{
    public:
        explicit Simulation(const Config& config);

        // Back to the doc 12.1 initial state. Placed food is kept (refilled) so the same
        // layout can be rerun and compared
        void reset();

        // Advances one tick in the doc 1.6 order: pheromone decay, ants, nest
        void tick();

        // Food can be placed before or during a run (doc 5.2, 14.1)
        void addFood(int x, int y);
        void clearFood();

        const Grid& getGrid() const { return grid; }
        const Nest& getNest() const { return nest; }
        const std::vector<Ant>& getAnts() const { return ants; }
        const std::vector<FoodSource>& getFood() const { return food; }
        const std::vector<StatSample>& getHistory() const { return history; }
        int getTick() const { return tickCount; }
        Outcome getOutcome() const { return outcome; }

        // Non-const so controls can change settings like toxicity for new food
        Config& config() { return cfg; }
        const Config& config() const { return cfg; }

    private:
        void recordSample();

        Config cfg;
        Grid grid;
        Nest nest;
        std::vector<Ant> ants;
        std::vector<FoodSource> food;           // live sources, depleted as ants take from them
        std::vector<FoodSource> foodLayout;     // sources as placed, restored on reset
        std::vector<StatSample> history;
        std::mt19937 rng;
        int tickCount = 0;
        Outcome outcome = Outcome::Running;
};
