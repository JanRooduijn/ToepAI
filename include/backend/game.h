#pragma once
#include <optional>
#include "trick.h"
#include "deck.h"
#include "ai.h"

class Player;
class Hand;
class AI;

class Game {
public:
    Game(const size_t& playerSize, const size_t& handSize);

    enum class State {
        INIT,
        PLAY,
        TOEP,
        TRICK_DONE,
        ROUND_DONE
    };

    Player* getPlayer(size_t index);
    size_t size() const;
    size_t getHandSize() const;
    Player* getCurrentPlayer() const;
    State getState() const;

    void update();

    void nextPlayer();
    void notifyAI();

    void toep(Player* player);
    void playToep(Player* player, bool call);
    void playCard(Player* player, size_t cardIndex);

    int getWinner();
    size_t getWager();
    std::optional<Card::Suit> getLeadingSuit();
    std::vector<Card*> getCardsInPlay() const;
    void unPause();

    bool isStartingToeper(Player* player);

private:
    std::vector<Player> players_;
    std::vector<AI> AIs_;
    std::vector<Trick> tricks_;
    const size_t handSize_;
    Deck deck_;

    State state_;
    Player* roundStarter_;
    Player* currentPlayer_;
    Player* toepStarter_;
    Player* currentToeper_;
    size_t wager_{1};
    size_t activePlayers_;
    Trick currentTrick_;

    void evaluateTrick();

    void closeRound();

    void evaluateToep();

    int winner_;
    size_t trick_;

    void dealHands();
    Hand dealHand();
    bool first_;

    bool paused{false};
    std::chrono::steady_clock::time_point lastActionTime;
    std::chrono::milliseconds pauseTime{1000};

    int getPlayerIndex(Player* player);
};
