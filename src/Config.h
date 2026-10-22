// Every tunable model perameter
#pragma once

struct Config {
    // World
    // Window is gridWidth * cellSize + panelWidth wide (1880) and gridHeight * cellSize tall (960),
    // sized to fit a 1920x1080 screen
    int gridWidth = 195;
    int gridHeight = 120;
    int cellSize = 8;                   // pixels per cell
    int panelWidth = 320;               // pixels for the stats panel right of the grid
    int targetFps = 60;
    float ticksPerSecond = 15.0f;       // model speed at 1x, separate from the frame rate (tune)
    int maxSpeed = 64;                  // fastest speed multiplier (64x)

    // Simulation
    unsigned int seed = 0;              // 0 = new random seed each run, anything else reproduces a run
    int statsSampleInterval = 10;       // record a graph point every N ticks

    // Ants
    int antCount = 100;                 // starting population
    float antMaxHealth = 100.0f;
    float antBaseSpeed = 1.0f;
    float antMinSpeedFraction = 0.15f;  // speed floor so dying ants still crawl (tune)
    float wanderTurnChance = 0.2f;      // chance per step a searching ant picks a new heading (tune)

    // Poison effect on ants
    float ingestDamage = 20.0f;         // health lost on pickup = toxicity * ingestDamage (tune)
    float carryDamagePerTick = 0.2f;    // health lost per tick carrying = toxicity * carryDamagePerTick (tune)

    // Food
    int foodAmount = 50;                // units of food per source
    float foodToxicity = 0.5f;          // 0 = harmless, 1 = max

    // Pheromones
    float pheromoneDeposit = 1.0f;      // dropped on each cell by a returning ant
    float pheromoneMax = 10.0f;         // cap per cell
    float pheromoneDecayRate = 0.01f;   // fraction lost per tick (tune)
    float pheromoneEpsilon = 0.01f;     // below this a cell is cleared to 0
    float followThreshold = 0.5f;       // searching ants ignore pheromone weaker than this
    float maxFollowChance = 0.9f;       // even strong trails are sometimes ignored (doc 14.2)

    // Nest
    // Nest health starts at antCount * nestTolerancePerAnt, so the poison threshold scales
    // with colony size. Each delivery removes toxicity * nestDoseMultiplier.
    float nestTolerancePerAnt = 0.5f;   // (tune)
    float nestDoseMultiplier = 1.0f;
    float nestRegenPerTick = 0.0f;      // not in the doc. > 0 lets the colony recover from small doses
};
