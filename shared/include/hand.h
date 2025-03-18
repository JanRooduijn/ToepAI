#pragma once
#include "card.h"
#include <vector>
#include <sstream>


class Hand {
public:
    Hand(std::vector<Card> cards);
    const Card& getCard(CardIndex cardIndex) const;
    void playCard(CardIndex cardIndex, size_t trickIndex);
    void setCard(CardIndex cardIndex, Card card);
    void win(CardIndex cardIndex);
    void lose(CardIndex cardIndex);
    void done(CardIndex cardIndex);
    size_t size() const;
    std::string toString() const;
    bool canPlay(Card::Suit suit) const;

    int maxFreeValue(Card::Suit suit) const;

    int averageValue() const;

private:
    std::vector<Card> cards_;
};

std::ostream& operator<<(std::ostream& out, const Hand& hand);

