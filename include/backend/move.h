#pragma once
#include "player.h"

class Move {
public:
    Move(Player* player, Card* card) : player_(player), card_(card), result_(0) {}
private:
    Player* player_;
    Card* card_;
    size_t result_;
};
