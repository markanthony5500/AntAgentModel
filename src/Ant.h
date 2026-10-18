#pragma once

#include <random>
#include <vector>

#include "Config.h"
#include "FoodSource.h"
#include "Grid.h"
#include "Nest.h"

// Ant agent

// Ant has two states, searching for food, or returning to the nest with food
enum class AntState { Searching, Returning };

// Returned by update, means ant has no deliver to the nest
const float NO_DELIVERY = -1.0f;

class Ant{
    public:
        // startDir: 0 up, 1 right, 2 down, 3 left.
        Ant(int x, int y, float maxHealth, int startDir);

        // Runs one tick for this ant, returns a toxin amount if ant has returned to the nest with food in this tick, else returns NO_DELIVERY (-1)
        float update(Grid& grid, std::vector<FoodSource>& food, const Nest& nest,
                     const Config& cfg, std::mt19937& rng);

        int x() const { return posX; }
        int y() const { return posY; }
        AntState getState() const { return state; }
        float getHealth() const { return health; }
        float getHealthFraction() const { return health / maxHealth; }
        bool isDead() const { return dead; }

    private:
        // Each step helper moves the ant at most one cell, one for a searching ant and one for a returning ant
        void stepSearching(Grid& grid, std::vector<FoodSource>& food, const Nest& nest,const Config& cfg, std::mt19937& rng);
        float stepReturning(Grid& grid, const Nest& nest, const Config& cfg);

        void wander(const Grid& grid, const Config& cfg, std::mt19937& rng);
        void moveTo(int nx, int ny);
        void takeDamage(float amount);

        int posX = 0;
        int posY = 0;
        int prevX = 0;                  // last cell, so trail following doesn't step straight back
        int prevY = 0;
        int dir = 0;                    // direction ant is heading in, ants have a chance to change direction (avoid crazy movement)
        float health = 0.0f;
        float maxHealth = 0.0f;
        float moveProgress = 0.0f;      // Use banking for movement, in order to have ants slow down
        float carriedToxicity = 0.0f;   // toxicity of carried food
        AntState state = AntState::Searching;
        bool dead = false;
};
