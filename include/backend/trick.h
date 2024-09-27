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

    std::vector<Player *> getPlayers() const;

    void setFold(Player* player, size_t fold);
    void setRaise(Player* player, size_t raise);
    Card::Suit getLeadingSuit() const;
    int getMaxValue() const;

    const std::vector<Move>& getMoves() const;

private:
    Card::Suit leadingSuit_ = Card::Suit::HEARTS;
    int maxValue_ = 0;
    std::vector<Move> moves_;
};



