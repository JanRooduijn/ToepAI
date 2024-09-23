#pragma once
#include <random>
#include <memory>
#include "game.h"

class Game;

class AI {
public:
    static AI& getInstance();
    void play(Game* game, size_t playerIndex);

    void toep(Game *game, size_t playerIndex);

private:
    AI() : g(rd()) {}
    AI(const AI&) = delete;
    AI& operator=(const AI&) = delete;
    std::random_device rd;
    std::mt19937 g;
};
