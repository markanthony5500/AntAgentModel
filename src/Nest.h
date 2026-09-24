#pragma once

// Nest representing ant colony health and ultimatly survival

class Nest{
    public:
        Nest() = default;
        Nest(int x, int y, float maxHealth, int population);

        int x() const { return posX; }
        int y() const { return posY; }
        bool isAt(int cx, int cy) const { return cx == posX && cy == posY; }

        // Returning ant drops off food. Poison lowers colony health
        void receiveDelivery(float toxin, float doseMultiplier);

        // Colony slowly recovers each tick, capped at max health
        void regenerate(float amount);

        void setPopulation(int count) { population = count; }

        float getHealth() const { return health; }
        float getMaxHealth() const { return maxHealth; }
        int getPopulation() const { return population; }
        int getFoodDelivered() const { return foodDelivered; }
        float getTotalPoisonDelivered() const { return totalPoisonDelivered; }
        bool isDead() const { return dead; }

    private:
        int posX = 0;
        int posY = 0;
        float health = 0.0f;
        float maxHealth = 0.0f;
        int population = 0;
        int foodDelivered = 0;
        float totalPoisonDelivered = 0.0f;
        bool dead = false;
};
