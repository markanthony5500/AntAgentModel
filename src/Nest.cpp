#include "Nest.h"

#include <algorithm>

Nest::Nest(int x, int y, float maxHealth, int population) {
    posX = x;
    posY = y;
    this->maxHealth = maxHealth;
    this->population = population;
    health = maxHealth;
}

void Nest::receiveDelivery(float toxin, float doseMultiplier) {
    foodDelivered++;
    totalPoisonDelivered += toxin;

    if (dead) return;

    health -= toxin * doseMultiplier;
    if (health <= 0.0f) {
        health = 0.0f;
        dead = true;
    }
}

void Nest::regenerate(float amount) {
    if (dead) return;
    health = std::min(health + amount, maxHealth);
}
