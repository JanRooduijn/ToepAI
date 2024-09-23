#include "../../include/backend/deck.h"

#include <random>
#include <__random/random_device.h>

Deck::Deck() { init(); }

void Deck::init() {
    cards.clear();
    for (int suit = Card::Suit::CLUBS; suit <= Card::Suit::SPADES; ++suit) {
        for (int value = Card::JACK; value <= 10; ++value) {
            cards.emplace_back(Card(value, static_cast<Card::Suit>(suit)));
        }
    }

}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
}

Card Deck::dealCard() {
    if (cards.size() == 0) throw std::invalid_argument("Deck is empty");
    Card card = cards.back();
    cards.pop_back();
    return card;
}
