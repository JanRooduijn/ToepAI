#pragma once
#include "move.h"
#include "player.h"
#include "card.h"
#include <vector>

class Player;
class Move;
class Card;

class Trick {
public:
    void addMove(const Move& move);
    Move* getStartingMove();
    size_t size();
    std::vector<Card*> getCards() const;
    void setOutcome(Player* player, size_t outcome);

private:
    std::vector<Move> moves_;
};



