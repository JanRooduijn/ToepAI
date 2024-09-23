#pragma once
#include <vector>
#include "move.h"

class Trick {
public:
    Trick(std::vector<Player>& players_, int startingPlayer);
    void addMove(const Move& move);

private:
    std::vector<Move> moves_;
};

