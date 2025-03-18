#pragma once
#include <optional>
#include "move.h"
#include "deck.h"
#include "ai.h"

class Player;
class Hand;
class AI;

using Trick = std::vector<Move>;

class Game {
public:
    Game(size_t playerSize, size_t handSize, bool server);

    enum class State {
        INIT,
        PLAY,
        TOEP,
        TRICK_DONE,
        ROUND_DONE
    };

    const Player& getPlayer(PlayerIndex index) const;
    size_t size() const;
    size_t aiPlayers() const;
    size_t getHandSize() const;
    State getState() const;

    bool update();
    void nextPlayer();
    void notifyAI();

    void toep(PlayerIndex);
    void playToep(PlayerIndex playerIndex, bool call);
    void playCard(PlayerIndex playerIndex, CardIndex cardIndex);
    void win(PlayerIndex playerIndex, CardIndex cardIndex);
    void lose(PlayerIndex playerIndex, CardIndex cardIndex);
    void done(PlayerIndex playerIndex, CardIndex cardIndex);

    void setCard(PlayerIndex playerIndex, CardIndex cardIndex, Card card);

    PlayerIndex getWinner() const;
    size_t getWager() const ;
    int getMaxValue() const ;
    std::optional<Card::Suit> getLeadingSuit();
    const std::vector<CardIndex> getCardsInPlay() const;
    void unPause();

    bool isStartingToeper(PlayerIndex playerIndex) const;

    const Trick& getCurrentTrick() const;
    size_t getTrickNo() const;

    void makeAI(PlayerIndex playerIndex);
    void makeHuman(PlayerIndex playerIndex);

    bool connected() const;
    void connect();
    void disconnect();

    PlayerIndex getRoundStarter() const;
    PlayerIndex getCurrentPlayer() const;
    PlayerIndex getToepStarter() const;
    PlayerIndex getCurrentToeper() const;

    void setState(State state);
    void setRoundStarter(PlayerIndex roundStarter);
    void setCurrentPlayer(PlayerIndex roundStarter);
    void setToepStarter(PlayerIndex roundStarter);
    void setCurrentToeper(PlayerIndex roundStarter);
    void setWager(size_t wager);

    void setScore(PlayerIndex playerIndex, size_t score);
    void setTrickNo(size_t trickNo);
    void setCurrentTrick(Trick trick);
    void reset(size_t playerSize);

private:
    bool server_;
    std::vector<Player> players_;
    std::vector<AI> AIs_;
    const size_t handSize_;
    Deck deck_;

    State state_;
    PlayerIndex roundStarter_;
    PlayerIndex currentPlayer_;
    PlayerIndex toepStarter_;
    PlayerIndex currentToeper_;
    size_t wager_;
    int maxValue_;
    size_t activePlayers_;
    size_t aiPlayers_;
    Trick currentTrick_;

    void evaluateTrick();
    void closeRound();
    void evaluateToep();

    int winner_;
    size_t trickNo_;

    void dealHands();
    Hand dealHand();
    bool first_;

    bool paused{true};
    std::chrono::steady_clock::time_point lastActionTime;
    std::chrono::milliseconds pauseTime{1000};

    bool connected_{false};
};
