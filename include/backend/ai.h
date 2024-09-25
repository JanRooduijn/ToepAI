#pragma once
#include "game.h"
#include <random>
#include <memory>

class Player;
class Game;

class AI {
public:
    static AI& getInstance();
    void play(Game* game, Player* player);
    void toep(Game *game, Player* player);

private:
    AI() : g(rd()) {}
    AI(const AI&) = delete;
    AI& operator=(const AI&) = delete;
    std::random_device rd;
    std::mt19937 g;
};
