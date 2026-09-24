#include "Grid.h"

#include <algorithm>

Grid::Grid(int width, int height) {
    w = width;
    h = height;
    pheromone.resize(w * h, 0.0f);
}

bool Grid::inBounds(int x, int y) const {
    return x >= 0 && x < w && y >= 0 && y < h;
}

float Grid::pheromoneAt(int x, int y) const {
    if (!inBounds(x, y)) return 0.0f;
    return pheromone[index(x, y)];
}

void Grid::deposit(int x, int y, float amount, float cap) {
    if (!inBounds(x, y)) return;
    float& p = pheromone[index(x, y)];
    p = std::min(p + amount, cap);
}

void Grid::decay(float rate, float epsilon) {
    const float keep = 1.0f - rate;
    for (float& p : pheromone) {
        p *= keep;
        if (p < epsilon) p = 0.0f;
    }
}

void Grid::clear() {
    std::fill(pheromone.begin(), pheromone.end(), 0.0f);
}
