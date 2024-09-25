#include "../../include/backend/trick.h"

#include <iostream>

void Trick::addMove(const Move& move) {
    moves_.push_back(move);
}

Move* Trick::getStartingMove() {
    if (moves_.empty()) return nullptr;

    return &moves_.front();
}

size_t Trick::size() {
    return moves_.size();
}

std::vector<Card*> Trick::getCards() const {
    std::vector<Card*> cards;
    cards.reserve(moves_.size()); // Reserve space to avoid reallocation

    for (const auto& move : moves_) {
        cards.push_back(move.getCard()); // Add the card pointer from each move
    }

    return cards;
}

void Trick::setOutcome(Player* player, size_t outcome) {
    for (Move& move : moves_) {
        if (move.getPlayer() == player) move.setOutcome(outcome);
    }
}

