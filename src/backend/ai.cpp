#include "../include/backend/ai.h"

AI& AI::getInstance() {
    static AI instance;
    return instance;
}


void AI::play(Game* game, Player* player) {
    Hand hand = player->getHand();

    std::uniform_int_distribution<> dis(0, game->size() - 1);
    size_t firstCard = dis(g);
    for (size_t i = 0; i < game->getHandSize(); ++i) {
        size_t cardIndex = (firstCard + i) % game->getHandSize();
        Card card = hand.getCard(cardIndex);

        if (card.state() != Card::State::INIT) continue;
        if (game->getLeadingSuit()) {
            Card::Suit leadingSuit = *game->getLeadingSuit();
            if (hand.canPlay(leadingSuit) && card.suit() != leadingSuit) continue;
        }

        game->playCard(player, cardIndex);
        break;
    }
}

void AI::toep(Game* game, Player* player) {
    std::bernoulli_distribution dist(0.5);
    bool call = dist(g);
    game->playToep(player, call);
}
