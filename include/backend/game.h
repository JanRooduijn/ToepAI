#pragma once
#include "ai.h"
#include "player.h"
#include "deck.h"

class AI;
class Player;

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

    Player& getPlayer(size_t index);
    size_t size() const;
    size_t getHandSize() const;
    size_t getCurrentPlayer() const;
    State getState() const;

    void update();

    void nextPlayer();
    void notifyAI();

    void toep(size_t playerIndex);

    void playToep(size_t playerIndex, bool call);

    void playCard(size_t playerIndex, size_t cardIndex);
    int getWinner();
    size_t getWager();
    std::optional<Card::Suit> getLeadingSuit();
    std::vector<Card*> getCardsInPlay() const;
    void unPause();

    bool isStartingToeper(size_t playerIndex);

private:
    std::vector<Player> players_;
    std::vector<AI> AIs_;
    std::vector<Card*> cardsInPlay_;
    const size_t handSize_;
    Deck deck_;

    State state_;
    size_t roundStarter_;
    size_t trickStarter_;
    size_t currentPlayer_;
    size_t toepStarter_;
    size_t currentToeper_;
    size_t wager_{1};
    size_t activePlayers_;

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

};
