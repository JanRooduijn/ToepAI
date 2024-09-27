#include "../include/backend/ai.h"

AI& AI::getInstance() {
    static AI instance;
    return instance;
}

void AI::play(Game* game, Player* player) {
    Hand& hand = player->getHand();
    const Trick& trick = game->getCurrentTrick();

    auto leadingSuit = game->getLeadingSuit();
    bool constrainedByLeadingSuit = leadingSuit && hand.canPlay(*leadingSuit);
    bool canWinAlreadyStartedTrick = trick.size() > 0 && hand.maxFreeValue(trick.getLeadingSuit()) > trick.getMaxValue();

    std::vector<size_t> eligibleIndices;

    for (size_t i = 0; i < hand.size(); ++i) {
        if (hand.getCard(i).state() != Card::State::INIT) continue;
        if (constrainedByLeadingSuit && hand.getCard(i).suit() != leadingSuit) continue;
        if (canWinAlreadyStartedTrick && hand.getCard(i).value() < trick.getMaxValue()) continue;
        eligibleIndices.push_back(i);
    }

    if (game->getTrickNo() < 2) {
        std::ranges::sort(eligibleIndices, [&](size_t a, size_t b) {
            return hand.getCard(a).value() < hand.getCard(b).value();
        });
    }
    else {
        std::ranges::sort(eligibleIndices, [&](size_t a, size_t b) {
            return hand.getCard(a).value() > hand.getCard(b).value();
        });
    }

    std::vector<size_t> candidateIndices;
    candidateIndices.push_back(eligibleIndices[0]);
    int prev = hand.getCard(eligibleIndices[0]).value();
    for (size_t k = 1; k < eligibleIndices.size(); ++k) {
        int curValue = hand.getCard(eligibleIndices[k]).value();
        if (abs(curValue - prev) > 1) break;
        candidateIndices.push_back(eligibleIndices[k]);
    }

    std::uniform_int_distribution<> dist(0, candidateIndices.size() - 1);
    size_t k = dist(g);
    game->playCard(player, candidateIndices[k]);
}

void AI::toep(Game* game, Player* player) {
    const auto& trick = game->getCurrentTrick();
    auto& hand = player->getHand();
    bool call = false;

    if (game-> getTrickNo() < 2) {
        std::bernoulli_distribution dist(hand.averageValue() / 10.0f);
        call = dist(g);
    }
    else {
        bool hasPlayed = false;
        for (const auto& playedPlayer : trick.getPlayers()) if (player == playedPlayer) hasPlayed = true;

        if (hasPlayed) {
            for (const auto& move : trick.getMoves()) {
                if (player == move.getPlayer() && move.getCard()->value() == trick.getMaxValue()) call = true;
            }
        }
        else if (hand.maxFreeValue(trick.getLeadingSuit()) > trick.getMaxValue()) call = true;
    }

    game->playToep(player, call);
}
