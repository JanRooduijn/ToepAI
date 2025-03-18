#include "../include/game.h"

#include <iostream>
#include <random>
#include <thread>
#include <__random/random_device.h>

Game::Game(size_t playerSize, size_t handSize, bool server) :
    handSize_(handSize), state_(Game::State::INIT),
    winner_(-1), trickNo_(0), first_(true), activePlayers_(0), aiPlayers_(playerSize - 1),
    wager_(1), maxValue_(0), server_(server) {
    if (playerSize < 2 || playerSize > 8) throw std::invalid_argument("Invalid player amount");
    if (handSize != 4 && handSize != 8) throw std::invalid_argument("Invalid hand size");
    if (handSize * playerSize > 32) throw std::invalid_argument("Not enough cards");

    deck_.shuffle();
    for (size_t i = 0; i < playerSize; ++i) {
        bool isAI = (i != 0);
        Hand hand = dealHand();
        players_.emplace_back(hand, isAI);
        activePlayers_++;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, players_.size() - 1);
    roundStarter_ = dis(g);
    currentPlayer_ = roundStarter_;

    lastActionTime = std::chrono::steady_clock::now() + pauseTime;
    paused = true;
}

Game::State Game::getState() const { return state_; }

bool Game::update() {
    if (connected_) return false;
    if (server_ || state_ != State::TRICK_DONE) {
        if (!players_[currentPlayer_].isParticipating()) unPause();
        if (std::chrono::steady_clock::now() - lastActionTime >= pauseTime) unPause();
    }
    if (paused) return false;

    switch (state_) {
        case State::INIT:
            state_ = State::PLAY;

        case State::PLAY:
            if (currentTrick_.size() > 0 && currentPlayer_ == currentTrick_.front().playerIndex) {
                evaluateTrick();
                state_ = State::TRICK_DONE;
            }
            else if (activePlayers_ == 1) state_ = State::TRICK_DONE;
            else if (!players_[currentPlayer_].isParticipating()) nextPlayer();
            else notifyAI();
        break;

        case State::TOEP:
            if (currentToeper_ == toepStarter_) evaluateToep();
            else if (!players_[currentToeper_].isParticipating()) nextPlayer();
            else notifyAI();
        break;

        case State::TRICK_DONE:
            // Mark all cards in the current trick as done
            for (const Move& move : currentTrick_) players_[move.playerIndex].done(move.cardIndex);

            if (activePlayers_ == 1 || trickNo_ == players_.size()) {
                closeRound();
                trickNo_ = 0;
                wager_ = 1;
                dealHands();
            }

            // Create a new Trick
            currentTrick_ = Trick();

            state_ = State::PLAY;
            break;
    }

    lastActionTime = std::chrono::steady_clock::now();
    paused = true;
    return true;
}

void Game::dealHands() {
    deck_.init();
    deck_.shuffle();
    activePlayers_ = 0;
    for (auto& player : players_) {
        player.setHand(dealHand());
        player.participate();
        activePlayers_++;
    }
}

Hand Game::dealHand() {
    std::vector<Card> cards;
    cards.reserve(handSize_);
    for (size_t j = 0; j < handSize_; ++j) cards.push_back(deck_.dealCard());
    Hand hand(std::move(cards));
    return hand;
}

const Player& Game::getPlayer(PlayerIndex playerIndex) const {
    if (playerIndex > players_.size() - 1) throw std::invalid_argument("Invalid index");
    return players_[playerIndex];
}
size_t Game::size() const { return players_.size(); }
size_t Game::aiPlayers() const { return aiPlayers_; }
size_t Game::getHandSize() const { return handSize_; }

void Game::nextPlayer() {
    switch (state_) {
        case State::PLAY:
            currentPlayer_ = (currentPlayer_ + 1) % players_.size();
            break;

        case State::TOEP:
            currentToeper_ = (currentToeper_ + 1) % players_.size();
            break;

        default:
            std::cerr << "Cannot switch to next player, because game is not in play or in toep." << std::endl;
    }

    lastActionTime = std::chrono::steady_clock::now();
    paused = true;
}

void Game::notifyAI() {
    if (connected_) // AI controlled by server
        return;

    switch (state_) {
        case State::PLAY:
            if (players_[currentPlayer_].isAI()) AI::getInstance().play(*this, currentPlayer_);
            break;
        case State::TOEP:
            if (players_[currentToeper_].isAI()) AI::getInstance().toep(*this, currentToeper_);
            break;
        default:
            break;
    }
}

void Game::toep(PlayerIndex playerIndex) {
    if (state_ != State::PLAY) {
        std::cerr << "Cannot toep because the game is not in play." << std::endl;
        return;
    }
    if (wager_ >= 3) { // Hardcoded max wager of 3
        std::cerr << "Cannot toep, because the maximum wager is hardcoded to be 3";
        return;
    }
    state_ = State::TOEP;
    toepStarter_= playerIndex;
    currentToeper_ = playerIndex;
    nextPlayer();
}

void Game::playToep(PlayerIndex playerIndex, bool call) { // perhaps later I will add the possibility to raise
    if (state_ != State::TOEP) {
        std::cerr << "cannot play toep because the game is not in toep." << std::endl;
        return;
    }
    if (currentToeper_ != playerIndex) {
        std::cerr << "cannot play toep because it is not Player" + std::to_string(playerIndex) + "'s turn." << std::endl;
        return;
    }
    if (!call) {
        auto hand = players_[playerIndex].getHand();
        for (int i = 0; i < hand.size(); ++i) {
            players_[playerIndex].done(i);
        }
        players_[playerIndex].addScore(wager_);
        players_[playerIndex].fold();
        activePlayers_--;
    }
    nextPlayer();
}

void Game::playCard(PlayerIndex playerIndex, CardIndex cardIndex) {
    if (state_ != State::PLAY) {
        std::cerr << "Cannot play card because the game is not in play." << std::endl;
        return;
    }
    if (currentPlayer_ != playerIndex) {
        std::cerr << "Cannot play card because it is not Player " <<  playerIndex << "'s turn." << std::endl;
        return;
    }
    int playerNo = 0;
    if (currentTrick_.size() > 0) playerNo = (playerIndex - currentTrick_.front().playerIndex) % size();
    if (playerNo < 0) playerNo += size();
    if (playerNo < currentTrick_.size()) {
        std::cerr << "Player " << playerIndex << " already played their card." << std::endl;
        return;
    }

    auto hand = players_[currentPlayer_].getHand();
    if (getLeadingSuit()) {
        Card::Suit leadingSuit = *getLeadingSuit();
        Card::Suit mySuit = hand.getCard(cardIndex).suit();
        if (mySuit != leadingSuit && hand.canPlay(leadingSuit)) {
            notifyAI();
            return;
        }
        maxValue_ = std::max(maxValue_, hand.getCard(cardIndex).value());
    }
    players_[playerIndex].playCard(cardIndex, currentTrick_.size());
    currentTrick_.emplace_back(playerIndex, cardIndex);
    nextPlayer();
}

void Game::setCard(PlayerIndex playerIndex, CardIndex cardIndex, Card card) { players_[playerIndex].setCard(cardIndex, card); }
void Game::win(PlayerIndex playerIndex, CardIndex cardIndex) { players_[playerIndex].win(cardIndex); }
void Game::lose(PlayerIndex playerIndex, CardIndex cardIndex) { players_[playerIndex].lose(cardIndex); }
void Game::done(PlayerIndex playerIndex, CardIndex cardIndex) { players_[playerIndex].done(cardIndex); }

void Game::evaluateTrick() {
    const Move& startingMove = currentTrick_.front();
    if (currentTrick_.size() == 0) {
        std::cerr << "evaluateTrick() called on empty trick";
        return;
    }

    Card::Suit suit = *getLeadingSuit();
    MoveIndex maxIndex = 0;
    auto moves = currentTrick_;
    for (MoveIndex i = 0; i < moves.size(); ++i) {
        Move& move = moves[i];
        PlayerIndex playerIndex = move.playerIndex;
        CardIndex cardIndex = move.cardIndex;
        const Card& card = players_[playerIndex].getHand().getCard(cardIndex);

        Move& maxMove = moves[maxIndex];
        PlayerIndex maxPlayerIndex = maxMove.playerIndex;
        CardIndex maxCardIndex = maxMove.cardIndex;
        const Card& maxCard = players_[maxPlayerIndex].getHand().getCard(maxCardIndex);

        if (card.suit() == suit && card.value() > maxCard.value()) maxIndex = i;
    }

    for (MoveIndex i = 0; i < moves.size(); ++i) {
        Move& move = moves[i];
        PlayerIndex playerIndex = move.playerIndex;
        CardIndex cardIndex = move.cardIndex;
        const Card& card = players_[playerIndex].getHand().getCard(cardIndex);

        if (i == maxIndex) players_[playerIndex].win(cardIndex);
        else players_[playerIndex].lose(cardIndex);
    }

    // calculate the winning player index
    int i = maxIndex;
    int pos = startingMove.playerIndex;
    while (i > 0) {
        if (players_[pos].isParticipating()) i--;
        pos = (pos + 1) % players_.size();
    }
    while (!players_[pos].isParticipating()) pos = (pos + 1) % players_.size();

    currentPlayer_ = pos;
    trickNo_++;
    if (trickNo_ == players_.size()) {
        for (PlayerIndex i = 0; i < players_.size(); ++i) {
            if (i != pos && players_[i].isParticipating()) {
                players_[i].addScore(wager_);
            }
        }
    }
}

void Game::closeRound() {
    currentPlayer_ = roundStarter_;
    nextPlayer();
    roundStarter_ = currentPlayer_;
}

void Game::evaluateToep() {
    wager_++;
    state_ = State::PLAY;
}

std::optional<Card::Suit> Game::getLeadingSuit() {
    if (currentTrick_.size() == 0) return std::nullopt;

    auto [playerIndex, cardIndex, fold, raise] = currentTrick_.front();
    return getPlayer(playerIndex).getHand().getCard(cardIndex).suit();
}

PlayerIndex Game::getWinner() const { return winner_; }
size_t Game::getWager() const { return wager_; }
int Game::getMaxValue() const { return maxValue_; }


const std::vector<CardIndex> Game::getCardsInPlay() const {
    const auto& moves = currentTrick_;
    std::vector<CardIndex> cards;
    cards.reserve(moves.size());
    for (const auto& move : currentTrick_) cards.push_back(move.cardIndex);
    return cards;
}

void Game::unPause() {
    if (std::chrono::steady_clock::now() - lastActionTime >= std::chrono::milliseconds(100)) paused = false;
}

bool Game::isStartingToeper(PlayerIndex playerIndex) const { return toepStarter_ == playerIndex; }
const Trick& Game::getCurrentTrick() const { return currentTrick_; }
size_t Game::getTrickNo() const { return trickNo_; }
void Game::makeAI(PlayerIndex playerIndex) {
    if (players_[playerIndex].isAI()) {
        std::cerr << "Player " + std::to_string(playerIndex) + " is already an AI." << std::endl;
    }
    else {
        players_[playerIndex].makeAI();
        aiPlayers_++;
    }
}
void Game::makeHuman(PlayerIndex playerIndex) {
    if (!players_[playerIndex].isAI()) {
        std::cerr << "Player " + std::to_string(playerIndex) + " is already human." << std::endl;
    }
    else {
        players_[playerIndex].makeHuman();
        aiPlayers_--;
    }
    players_[playerIndex].makeHuman();
}

bool Game::connected() const { return connected_; }
void Game::connect() {
    if (connected_) {
        std::cerr << "Connected already!" << std::endl;
        return;
    }

    connected_ = true;
}
void Game::disconnect() {
    if (!connected_) {
        std::cerr << "Disconnected already!" << std::endl;
        return;
    }

    else connected_ = false;
}

PlayerIndex Game::getRoundStarter() const  { return roundStarter_; }
PlayerIndex Game::getCurrentPlayer() const { return currentPlayer_; }
PlayerIndex Game::getToepStarter() const { return toepStarter_; }
PlayerIndex Game::getCurrentToeper() const { return currentToeper_; }

void Game::setState(State state) { state_ = state; }
void Game::setRoundStarter(PlayerIndex roundStarter) { roundStarter_ = roundStarter; }
void Game::setCurrentPlayer(PlayerIndex currentPlayer) { currentPlayer_ = currentPlayer; }
void Game::setToepStarter(PlayerIndex toepStarter) { toepStarter_ = toepStarter; }
void Game::setCurrentToeper(PlayerIndex currentToeper) { currentToeper_ = currentToeper; }
void Game::setWager(size_t wager) { wager_ = wager; }

void Game::setScore(PlayerIndex playerIndex, size_t score) { players_[playerIndex].setScore(score); }
void Game::setTrickNo(size_t trickNo) { trickNo_ = trickNo; }
void Game::setCurrentTrick(Trick trick) { currentTrick_ = trick; }

void Game::reset(size_t playerSize) {
    state_ = State::INIT;
    winner_ = -1;
    first_ = true;
    trickNo_ = 0;
    activePlayers_ = 0;
    aiPlayers_ = playerSize - 1;
    wager_ = 1;
    maxValue_ = 0;
    players_.clear();
    AIs_.clear();
    currentTrick_.clear();
    trickNo_ = 0;
    paused = true;
    connected_ = false;

    if (playerSize < 2 || playerSize > 8) throw std::invalid_argument("Invalid player amount");
    if (handSize_ * playerSize > 32) throw std::invalid_argument("Not enough cards");

    deck_.shuffle();
    for (size_t i = 0; i < playerSize; ++i) {
        bool isAI = (i != 0);
        Hand hand = dealHand();
        players_.emplace_back(hand, isAI);
        activePlayers_++;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, players_.size() - 1);
    roundStarter_ = dis(g);
    currentPlayer_ = roundStarter_;

    lastActionTime = std::chrono::steady_clock::now() + pauseTime;
    paused = true;
}