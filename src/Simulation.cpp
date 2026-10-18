#include "Simulation.h"


Simulation::Simulation(const Config& config): cfg(config), grid(config.gridWidth, config.gridHeight) {
    // Set rng, using seed if provided
    rng.seed(cfg.seed == 0 ? std::random_device{}() : cfg.seed);

    reset();
}

void Simulation::reset() {
    // Back to the doc 12.1 initial state
    grid.clear();
    history.clear();
    tickCount = 0;
    outcome = Outcome::Running;

    // Nest sits at the grid center. Its health scales with colony size
    int nestX = grid.width() / 2;
    int nestY = grid.height() / 2;
    nest = Nest(nestX, nestY, cfg.antCount * cfg.nestTolerancePerAnt, cfg.antCount);

    // Every ant starts on the nest facing a random direction so they fan out
    std::uniform_int_distribution<int> randomDir(0, 3);
    ants.clear();
    ants.reserve(cfg.antCount);
    for (int i = 0; i < cfg.antCount; ++i) {
        ants.push_back(Ant(nestX, nestY, cfg.antMaxHealth, randomDir(rng)));
    }

    // Refill food from the placed layout so runs are comparable
    food = foodLayout;

    // Graph starts at tick 0
    recordSample();
}

void Simulation::tick() {
    // Make sure we are running
    if(outcome != Outcome::Running){
        return;
    }

    // Decay pheremones
    grid.decay(cfg.pheromoneDecayRate, cfg.pheromoneEpsilon);

    // Update every ant, store delivered toxin amount and alive ants
    std::vector<float> pendingDeliveries;
    std::vector<Ant> aliveAnts;

    for(Ant& ant : ants){
        float toxin = ant.update(grid, food, nest, cfg, rng);

        if(toxin != NO_DELIVERY){
            pendingDeliveries.push_back(toxin);
        }

        if(ant.getHealth() > 0){
            aliveAnts.push_back(ant);
        }
    }

    // Update ants to remove dead ants
    ants = aliveAnts;

    // Update nest population size
    nest.setPopulation(ants.size());

    // Give the poision delivery to nest, if any
    for(float toxin : pendingDeliveries){
        nest.receiveDelivery(toxin, cfg.nestDoseMultiplier);
    }

    // Regenerate nest
    nest.regenerate(cfg.nestRegenPerTick);
    
    // Advance the tick, and record a graph point if a sample interval is hit
    ++tickCount;

    if(tickCount % cfg.statsSampleInterval == 0){
        recordSample();
    }

    // Check if the run is over
    if(nest.isDead()){
        outcome = Outcome::NestPoisoned;
    }
    else if(ants.empty()){
        outcome = Outcome::AllAntsDead;
    }

    // Record the final point so the graph shows how the run ended, unless interval is this tick
    if(outcome != Outcome::Running && tickCount % cfg.statsSampleInterval != 0){
        recordSample();
    }
}

void Simulation::addFood(int x, int y) {
    // Food can't go off the grid or on top of the nest
    if(!grid.inBounds(x, y) || nest.isAt(x, y)){
        return;
    }

    FoodSource source = {x, y, cfg.foodAmount, cfg.foodToxicity};

    // Add this food source
    food.push_back(source);
    foodLayout.push_back(source);   // Allows us to remember layout on reset if needed
}

void Simulation::clearFood() {
    food.clear();
    foodLayout.clear();
}

void Simulation::recordSample() {
    // One point on the stats graph
    StatSample sample = {tickCount, nest.getPopulation(), nest.getHealth()};
    history.push_back(sample);
}
