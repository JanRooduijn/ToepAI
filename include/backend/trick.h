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
    size_t size() const;
    std::vector<Card*> getCards() const;
    void setFold(Player* player, size_t fold);
    void setRaise(Player* player, size_t raise);


private:
    std::vector<Move> moves_;
};



