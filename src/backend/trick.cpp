#include "../../include/backend/trick.h"

#include <iostream>

void Trick::addMove(const Move& move) {
    if (maxValue_ == 0) {
        leadingSuit_ = move.getCard()->suit();
        maxValue_ = move.getCard()->value();
    }
    else if (move.getCard()->suit() == leadingSuit_) maxValue_ = std::max(maxValue_, move.getCard()->value());
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

std::vector<Player*> Trick::getPlayers() const {
    std::vector<Player*> players;
    players.reserve(moves_.size()); // Reserve space to avoid reallocation

    for (const auto& move : moves_) {
        players.push_back(move.getPlayer()); // Add the card pointer from each move
    }

    return players;
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

Card::Suit Trick::getLeadingSuit() const { return leadingSuit_; }
int Trick::getMaxValue() const { return maxValue_; }
const std::vector<Move>& Trick::getMoves() const { return moves_; }




