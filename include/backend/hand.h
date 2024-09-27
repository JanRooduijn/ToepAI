#pragma once
#include "card.h"
#include <vector>
#include <sstream>

class Hand {
public:
    Hand(std::vector<Card>&);
    Card& getCard(size_t index);
    void playCard(size_t index);
    size_t size() const;
    [[nodiscard]] std::string toString() const;
    bool canPlay(Card::Suit suit) const;

    int maxFreeValue(Card::Suit suit) const;

    int averageValue() const;

private:
    std::vector<Card> cards_;
};

std::ostream& operator<<(std::ostream& out, const Hand& hand);

