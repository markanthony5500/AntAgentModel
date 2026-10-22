#include "Ant.h"

#include <algorithm>
#include <cstdlib>

// Offsets to 4 neighboring cells (DX[0] DY[0] = bottom, DX[1] DY[1] = up, ect...)
static const int DX[4] = {0, 1, 0, -1};
static const int DY[4] = {-1, 0, 1, 0};

// Returns the food source at (x, y) that still has food left, or nullptr
static FoodSource* foodAt(std::vector<FoodSource>& food, int x, int y) {
    for (FoodSource& source : food) {
        if (source.x == x && source.y == y && !source.isDepleted()) {
            return &source;
        }
    }

    return nullptr;
}

Ant::Ant(int x, int y, float maxHealth, int startDir) {
    posX = x;
    posY = y;
    prevX = x;      // no previous cell yet, so trail following has nothing to exclude
    prevY = y;
    dir = startDir;
    this->maxHealth = maxHealth;
    health = maxHealth;
}

float Ant::update(Grid& grid, std::vector<FoodSource>& food, const Nest& nest,
                  const Config& cfg, std::mt19937& rng) {
    if (dead) return NO_DELIVERY;

    // If the ant is returning to the nest, it takes damage, check agian if dead
    if (state == AntState::Returning) {
        takeDamage(carriedToxicity * cfg.carryDamagePerTick);
        if (dead) return NO_DELIVERY;
    }

    // Ant speed should slow as its health gets lower
    float speed = cfg.antBaseSpeed * std::max(cfg.antMinSpeedFraction, getHealthFraction());
    moveProgress += speed;

    // We bank movement, so as long as progress is above 1, we move until we get below 1
    while (moveProgress >= 1.0f && !dead) {
        moveProgress -= 1.0f;

        if (state == AntState::Searching) {
            stepSearching(grid, food, nest, cfg, rng);
        }
        else {
            float toxin = stepReturning(grid, nest, cfg);
            if (toxin != NO_DELIVERY) {
                return toxin;   // dropping food off ends the ant's turn
            }
        }
    }

    return NO_DELIVERY;
}

void Ant::stepSearching(Grid& grid, std::vector<FoodSource>& food, const Nest& nest,
                        const Config& cfg, std::mt19937& rng) {

    // 1: FOOD CHECK

    // If food is in a nearby cell(up/down/left/right) move towards it
    FoodSource* source = foodAt(food, posX, posY);
    if (!source) {
        // Check neighbors
        for (int d = 0; d < 4; ++d) {
            source = foodAt(food, posX + DX[d], posY + DY[d]);
            if (source) {
                moveTo(source->x, source->y);
                break;
            }
        }
    }

    // Take food and return if we found any (YUMMY!)
    if (source) {
        source->take();
        state = AntState::Returning;
        carriedToxicity = source->toxicity;
        takeDamage(source->toxicity * cfg.ingestDamage);    // dose from grabbing it
        return;
    }

    // 2: PHEREMONE CHECK
    // Check Neighboring cells for pheremone strength, and get the direction ant is inclined to move towards
    int nestDist = std::abs(posX - nest.x()) + std::abs(posY - nest.y());
    int bestDir = -1;
    float bestStrength = 0.0f;  // Holds the highest pheremone strength

    // Check neighbors
    for (int d = 0; d < 4; ++d) {
        int nx = posX + DX[d];
        int ny = posY + DY[d];
        if (!grid.inBounds(nx, ny) || (nx == prevX && ny == prevY)) continue;

        int dist = std::abs(nx - nest.x()) + std::abs(ny - nest.y());
        if (dist <= nestDist) continue; // We want to go away from the nest

        float strength = grid.pheromoneAt(nx, ny);
        if (strength > bestStrength) {
            bestStrength = strength;
            bestDir = d;
        }
    }

    // Stronger trails are more likely to be followed
    if (bestDir != -1 && bestStrength >= cfg.followThreshold) {
        // turn highest strenght neighboring cell into a number 0-1 based on max strength
        // Multiply that 0-1 number by the highest follow chance te get the chance ant moves in that direction
        float chance = cfg.maxFollowChance * (bestStrength / cfg.pheromoneMax);

        // Already on a trail: keep following it. Off a trail: roll to see if the ant joins it
        // (stronger trail, means more likely). Rolling every step would make ants fall off after a few cells
        std::uniform_real_distribution<float> roll(0.0f, 1.0f);
        bool onTrail = grid.pheromoneAt(posX, posY) >= cfg.followThreshold;
        if (onTrail || roll(rng) < chance) {
            dir = bestDir;
            moveTo(posX + DX[dir], posY + DY[dir]);
            return;
        }
    }

    // 3: NOTHING FOUND, WANDER

    // No trail, no food, just wander
    wander(grid, cfg, rng);
}

float Ant::stepReturning(Grid& grid, const Nest& nest, const Config& cfg) {
    // Ant knows where the nest is, move towards it

    // Drop a pheremone at current location
    grid.deposit(posX, posY, cfg.pheromoneDeposit, cfg.pheromoneMax);

    // Get x, y distance from nest
    int dx = nest.x() - posX;
    int dy = nest.y() - posY;

    // Move whatever direction gets you closer to nest
    if (std::abs(dx) >= std::abs(dy)) {
        moveTo(posX + (dx > 0 ? 1 : -1), posY);
    }
    else {
        moveTo(posX, posY + (dy > 0 ? 1 : -1));
    }

    // Made it to nest, reset carried toxicity and return delivered toxin amount
    if (nest.isAt(posX, posY)) {
        float delivered = carriedToxicity;
        carriedToxicity = 0.0f;
        state = AntState::Searching;
        return delivered;
    }

    return NO_DELIVERY;
}

void Ant::wander(const Grid& grid, const Config& cfg, std::mt19937& rng) {
    // Usually keep the same heading, sometimes pick a new random one (doc 3.2, 11.1).
    // Picking a new direction every step would make ants jitter in place instead of spreading out
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    std::uniform_int_distribution<int> randomDir(0, 3);

    // Check and see if ant turns
    if (roll(rng) < cfg.wanderTurnChance) {
        dir = randomDir(rng);
    }

    // Get next pos
    int nx = posX + DX[dir];
    int ny = posY + DY[dir];

    // Ant is going to move out of grid, get new random direction and dont move
    if (!grid.inBounds(nx, ny)) {
        dir = randomDir(rng);
        return;
    }

    moveTo(nx, ny);
}

void Ant::moveTo(int nx, int ny) {
    // Remember where we came from so trail following doesn't step straight back
    prevX = posX;
    prevY = posY;
    posX = nx;
    posY = ny;
}

void Ant::takeDamage(float amount) {
    health -= amount;
    if (health <= 0.0f) {
        health = 0.0f;
        dead = true;
    }
}
