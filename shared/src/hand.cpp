#include "../include/hand.h"

Hand::Hand(std::vector<Card> cards) : cards_(std::move(cards)) {
    if (cards_.size() < 4 || cards_.size() > 8) throw std::invalid_argument("Number of cards should be between 4 and 8");
}

const Card& Hand::getCard(CardIndex cardIndex) const {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    return cards_[cardIndex];
}

void Hand::setCard(CardIndex cardIndex, Card card)  {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[cardIndex] = card;
}

void Hand::playCard(CardIndex cardIndex, size_t trickIndex) {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[cardIndex].play(trickIndex);
}

void Hand::win(CardIndex cardIndex) {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[cardIndex].win();
}

void Hand::lose(CardIndex cardIndex) {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[cardIndex].lose();
}

void Hand::done(CardIndex cardIndex) {
    if (cardIndex > cards_.size() - 1) throw std::invalid_argument("Invalid index");
    cards_[cardIndex].done();
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
    for (const auto& card : cards_) {
        if (card.state() == Card::State::INIT && card.suit() == suit) {
            return true;
        }
    }
    return false;
}

int Hand::maxFreeValue(Card::Suit suit) const {
    int maxValue = 0;
    for (const auto& card : cards_) {
        if (card.state() == Card::State::INIT && card.suit() == suit) {
            maxValue = std::max(maxValue, card.value());
        }
    }
    return maxValue;
}

int Hand::averageValue() const {
    int sum = 0;
    for (const auto& card : cards_) sum += card.value();
    return sum / cards_.size();
}