#pragma once

#include <vector>

// Main 2D lattice representing pheremone locations

class Grid{
    public:
        Grid(int width, int height);

        int width() const { return w; }
        int height() const { return h; }

        bool inBounds(int x, int y) const;

        // Returns 0 outside the grid so ants can check neighbors without bounds tests
        float pheromoneAt(int x, int y) const;

        // Deposit pheremone trail at x,y locaiton. Need to pass in amount and the max amount of pheremones at a location
        void deposit(int x, int y, float amount, float cap);

        // Pheremones should decay each tick
        // Need epsilon here to solve for pheremone trail aproaching 0
        void decay(float rate, float epsilon = 0.01f);
        void clear();

    private:
        int w;
        int h;
        std::vector<float> pheromone;   // flat w*h array representing 2D grid
        int index(int x, int y) const { return y * w + x; } // Used to access 1D array as 2D using x,y cords
};
