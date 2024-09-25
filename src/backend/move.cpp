#include "../../include/backend/move.h"

Player* Move::getPlayer() const {
    return player_;
}

Card* Move::getCard() const {
    return card_;
}

void Move::setOutcome(size_t outcome) {
    outcome_ = outcome;
}


