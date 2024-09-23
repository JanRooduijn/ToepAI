#pragma once
#include <vector>
#include "player.h"

class Trick {
public:
    Trick(std::vector<Player>& players_, int startingPlayer);
    void playCard(Card* card);
    

private:
    std::vector<Card*> cardsInPlay_;
    std::vector<Player>& players_;

    size_t startingPlayer_;
    size_t currentPlayer_;
};

