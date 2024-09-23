#include "../../include/backend/hand.h"

Hand::Hand(std::vector<Card>& cards) : cards_(cards) {
    if (cards.size() < 4 || cards.size() > 8) throw std::invalid_argument("Number of cards should be between 4 and 8");
}

Card& Hand::getCard(size_t index) {
    if (index > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    return cards_[index];
}

void Hand::playCard(size_t index) {
    if (index > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[index].play();
}

size_t Hand::size() const {
    return cards_.size();
}

std::ostream& operator<<(std::ostream& out, Hand& hand) {
    out << "[";
    for (size_t i = 0; i < hand.size(); i++) {
        if (i != 0) out << ", ";
        out << hand.getCard(i);
    }
    out << "]";
    return out;
}

bool Hand::canPlay(Card::Suit suit) const {
    for (size_t i = 0; i < size(); ++i) {
        if (cards_[i].state() == Card::State::INIT && cards_[i].suit() == suit) {
            return true;
        }
    }
    return false;
}
