#pragma once
#include "card.h"
#include <vector>

class Deck {
public:
    Deck();

    void init();

    void shuffle();
    Card dealCard();

private:
    std::vector<Card> cards;
};

