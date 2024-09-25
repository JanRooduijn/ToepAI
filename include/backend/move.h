#pragma once
#include "player.h"
#include "card.h"

class Move {
public:
    Move(Player* player, Card* card) : player_(player), card_(card), outcome_(0) {}
    Player* getPlayer() const;
    Card* getCard() const;
    void setOutcome(size_t outcome);
private:
    Player* player_;
    Card* card_;
    size_t outcome_;
};
