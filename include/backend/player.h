#pragma once
#include "hand.h"
#include "ai.h"
#include <memory>

class Player {
public:
    Player(Hand& hand, bool isAI);

    Hand& getHand();
    void setHand(const Hand &hand);

    size_t score() const;
    void addScore(size_t n);
    bool isAI() const;
    void playCard(size_t cardIndex);
    bool isParticipating();
    void participate();
    void fold();

private:
    Hand hand_;
    size_t score_;
    bool isAI_;
    bool participating_{true};
};
