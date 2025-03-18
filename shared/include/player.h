#pragma once
#include "hand.h"
#include <memory>

class Player {
public:
    Player(Hand& hand, bool isAI);

    const Hand& getHand() const;
    void setHand(Hand hand);

    size_t score() const;
    void addScore(size_t score);
    void setScore(size_t score);
    bool isAI() const;
    void playCard(CardIndex cardIndex, size_t trickIndex);
    void setCard(CardIndex cardIndex, Card card);
    void win(CardIndex cardIndex);
    void lose(CardIndex cardIndex);
    void done(CardIndex cardIndex);
    bool isParticipating();
    void participate();
    void fold();
    size_t getWager() const { return wager_; }
    void setWager(size_t wager) { wager_ = wager; }
    void makeHuman();
    void makeAI();


private:
    Hand hand_;
    size_t score_;
    bool isAI_;
    bool participating_{true};
    size_t wager_;
};

using PlayerIndex = std::vector<Player>::size_type;
