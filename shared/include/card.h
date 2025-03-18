#pragma once
#include <string>
#include <iostream>

class Card {
public:
    static constexpr int JACK = 3;
    static constexpr int QUEEN = 4;
    static constexpr int KING = 5;
    static constexpr int ACE = 6;

    enum Suit {
        CLUBS,
        DIAMONDS,
        HEARTS,
        SPADES
    };

    enum class State {
        INIT,
        IN_PLAY,
        DONE
    };

    Card(int value, Suit suit);
    Card(int value, Suit suit, State state, bool won, size_t trickIndex);
    Card(const Card& o);

    bool isValid() const;
    std::string toString() const;

    bool isRed() const;
    int value() const;
    Suit suit() const;
    State state() const;
    void win();
    void lose();
    void done();
    bool won() const;
    void play(size_t trickIndex);
    size_t getTrickIndex() const;

    void setValue(int value);
    void setSuit(Suit suit);

private:
    int value_;
    Suit suit_;
    State state_;
    bool won_{false};
    size_t trickIndex_{0};
};

using CardIndex = std::vector<Card>::size_type;

std::ostream& operator<<(std::ostream& out, const Card& card);

