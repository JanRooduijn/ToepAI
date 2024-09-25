#include "../../include/backend/move.h"

Player* Move::getPlayer() const {
    return player_;
}

Card* Move::getCard() const {
    return card_;
}

void Move::setFold(size_t fold) {
    fold_ = fold;
}

void Move::setRaise(size_t raise) {
    raise_ = raise;
}

