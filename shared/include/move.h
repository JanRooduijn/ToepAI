#pragma once
#include "player.h"
#include "card.h"

struct Move {
    PlayerIndex playerIndex;
    CardIndex cardIndex;
    size_t fold;
    size_t raise;
};

using MoveIndex = std::vector<Card>::size_type;
