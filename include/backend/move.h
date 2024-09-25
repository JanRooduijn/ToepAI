#pragma once
#include "player.h"
#include "card.h"

class Move {
public:
    Move(Player* player, Card* card) : player_(player), card_(card), fold_(0), raise_(0) {}
    Player* getPlayer() const;
    Card* getCard() const;
    void setFold(size_t outcome);
    void setRaise(size_t raise);
private:
    Player* player_;
    Card* card_;
    size_t fold_;
    size_t raise_;
};
