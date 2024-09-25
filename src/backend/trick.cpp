#include "../../include/backend/trick.h"

#include <iostream>

void Trick::addMove(const Move& move) {
    moves_.push_back(move);
}

Move* Trick::getStartingMove() {
    if (moves_.empty()) return nullptr;

    return &moves_.front();
}

size_t Trick::size() const {
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

void Trick::setFold(Player* player, size_t fold) {
    for (Move& move : moves_) {
        if (move.getPlayer() == player) move.setFold(fold);
    }
}

void Trick::setRaise(Player* player, size_t raise) {
    for (Move& move : moves_) {
        if (move.getPlayer() == player) move.setRaise(raise);
    }
}


