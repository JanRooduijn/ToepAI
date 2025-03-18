#include "../include/ai.h"

AI& AI::getInstance() {
    static AI instance;
    return instance;
}

void AI::play(Game& game, PlayerIndex playerIndex) {
    const Hand& hand = game.getPlayer(playerIndex).getHand();
    const Trick& trick = game.getCurrentTrick();

    auto leadingSuit = game.getLeadingSuit();
    bool constrainedByLeadingSuit = leadingSuit && hand.canPlay(*leadingSuit);
    bool canWinAlreadyStartedTrick = trick.size() > 0 && hand.maxFreeValue(*game.getLeadingSuit()) > game.getMaxValue();

    std::vector<CardIndex> eligibleIndices;

    for (CardIndex i = 0; i < hand.size(); ++i) {
        if (hand.getCard(i).state() != Card::State::INIT) continue;
        if (constrainedByLeadingSuit && hand.getCard(i).suit() != leadingSuit) continue;
        if (canWinAlreadyStartedTrick && hand.getCard(i).value() < game.getMaxValue()) continue;
        eligibleIndices.push_back(i);
    }

    if (game.getTrickNo() < 2) {
        std::ranges::sort(eligibleIndices, [&](CardIndex a, CardIndex b) {
            return hand.getCard(a).value() < hand.getCard(b).value();
        });
    }
    else {
        std::ranges::sort(eligibleIndices, [&](CardIndex a, CardIndex b) {
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
    game.playCard(playerIndex, candidateIndices[k]);
}

void AI::toep(Game& game, PlayerIndex playerIndex) {
    const auto& trick = game.getCurrentTrick();
    auto& hand = game.getPlayer(playerIndex).getHand();
    bool call = false;

    if (game.getTrickNo() < 2) {
        std::bernoulli_distribution dist(hand.averageValue() / 10.0f);
        call = dist(g);
    }
    else {
        bool hasPlayed = false;
        for (const auto& move : trick) if (playerIndex == move.playerIndex) hasPlayed = true;

        if (hasPlayed) {
            for (const auto& move : trick) {
                int value = game.getPlayer(move.playerIndex).getHand().getCard(move.cardIndex).value();
                if (playerIndex == move.playerIndex && value == game.getMaxValue()) call = true;
            }
        }
        else if (hand.maxFreeValue(*game.getLeadingSuit()) > game.getMaxValue()) call = true;
    }

    game.playToep(playerIndex, call);
}
