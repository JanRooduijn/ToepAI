#include "../../include/backend/card.h"
#include <sstream>

Card::Card(const int value, const Suit suit) : value_(value), suit_(suit), state_(State::INIT) {}
Card::Card(const Card& o) = default;

bool Card::isValid() const { return ((value_ >= 3) && (value_ <= 10)); }

std::string Card::toString() const {
    std::ostringstream out;
    out << *this;
    return out.str();
}

bool Card::isRed() const { return ((suit_ == Suit::HEARTS) || (suit_ == Suit::DIAMONDS)); }
int Card::value() const { return value_; }
Card::Suit Card::suit() const { return suit_; }
Card::State Card::state() const { return state_; }
void Card::play() {
    if (state_ != State::INIT) return; // maybe log this?
    state_ = State::IN_PLAY;
}
void Card::win() {
    won_ = true;
}
void Card::lose() {
    won_ = false;
}
void Card::done() {
    state_ = State::DONE;
}

bool Card::won() const { return won_; }



std::ostream& operator<<(std::ostream& out, const Card& card) {
    if (!card.isValid()) {
        out << "<Invalid>";
        return out;
    }

    switch (int value = card.value()) {
        case Card::JACK: out << "Jack"; break;
        case Card::QUEEN: out << "Queen"; break;
        case Card::KING: out << "King"; break;
        case Card::ACE: out << "Ace"; break;
        default: out << value; break;
    }

    out << " of ";

    switch (card.suit()) {
        case Card::Suit::SPADES: out << "Spades"; break;
        case Card::Suit::CLUBS: out << "Clubs"; break;
        case Card::Suit::HEARTS: out << "Hearts"; break;
        case Card::Suit::DIAMONDS: out << "Diamonds"; break;
    }

    return out;
}






