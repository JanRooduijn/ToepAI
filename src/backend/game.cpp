#include "../include/backend/game.h"

#include <iostream>
#include <random>
#include <thread>
#include <__random/random_device.h>

Game::Game(const size_t& playerSize, const size_t& handSize) :
    handSize_(handSize), state_(Game::State::INIT),
    winner_(-1), trick_(0), first_(true), activePlayers_(0) {
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
    trickStarter_ = roundStarter_;
    currentPlayer_ = roundStarter_;
}

Game::State Game::getState() const { return state_; }

void Game::update() {
    if (state_ != State::TRICK_DONE && state_ != State::ROUND_DONE) {
        if (!players_[currentPlayer_].isParticipating()) unPause();
        if (std::chrono::steady_clock::now() - lastActionTime >= pauseTime) unPause();
    }
    if (paused) return;

    switch (state_) {
        case State::INIT:
            state_ = State::PLAY;

        case State::PLAY:
            if (currentPlayer_ == trickStarter_ && cardsInPlay_.size() > 0) {
                evaluateTrick();
                state_ = State::TRICK_DONE;
            }
            else if (activePlayers_ == 1 || trick_ == players_.size()) {
                closeRound();
                state_ = State::ROUND_DONE;
            }
            else if (!players_[currentPlayer_].isParticipating()) nextPlayer();
            else notifyAI();
            break;

        case State::TOEP:
            if (currentToeper_ == toepStarter_) state_ = State::PLAY;
            else if (!players_[currentToeper_].isParticipating()) nextPlayer();
            else notifyAI();
            break;

        case State::TRICK_DONE:
        case State::ROUND_DONE:
            for (Card* card : cardsInPlay_) card->done();
            cardsInPlay_.clear();
            if (state_ == State::ROUND_DONE) {
                trick_ = 0;
                wager_ = 1;
                dealHands();
            }
            state_ = State::PLAY;
            break;
    }

    lastActionTime = std::chrono::steady_clock::now();
    paused = true;
}

void Game::dealHands() {
    deck_.init();
    deck_.shuffle();
    activePlayers_ = 0;
    for (size_t i = 0; i < players_.size(); ++i) {
        players_[i].setHand(dealHand());
        players_[i].participate();
        activePlayers_++;
    }
}

Hand Game::dealHand() {
    std::vector<Card> cards;
    for (size_t j = 0; j < handSize_; ++j) {
        cards.push_back(deck_.dealCard());
    }
    Hand hand(cards);
    return hand;
}

Player& Game::getPlayer(size_t index) {
    if (index > players_.size() - 1) throw std::invalid_argument("Invalid index");
    return players_[index];
}
size_t Game::size() const { return players_.size(); }
size_t Game::getHandSize() const { return handSize_; }
size_t Game::getCurrentPlayer() const { return currentPlayer_; }

void Game::nextPlayer() {
    switch (state_) {
        case State::PLAY:
            currentPlayer_ = (currentPlayer_ + 1) % players_.size();
            break;

        case State::TOEP:
            currentToeper_ = (currentToeper_ + 1) % players_.size();
            if (currentToeper_ == toepStarter_) {
                state_ = State::PLAY;
                wager_++;
            }
            break;

        default:
            std::cerr << "Cannot switch to next player, because game is not in play or in toep.";
    }

    lastActionTime = std::chrono::steady_clock::now();
    paused = true;
}

void Game::notifyAI() {
    switch (state_) {
        case State::PLAY:
            if (players_[currentPlayer_].isAI()) AI::getInstance().play(this, currentPlayer_);
            break;
        case State::TOEP:
            if (players_[currentToeper_].isAI()) AI::getInstance().toep(this, currentToeper_);
            break;
        default:
            break;
    }
}

void Game::toep(size_t playerIndex) {
    if (state_ != State::PLAY) {
        std::cerr << "cannot toep because the game is not in play." << std::endl;
        return;
    }
    state_ = State::TOEP;
    toepStarter_= playerIndex;
    nextPlayer();
}

void Game::playToep(size_t playerIndex, bool call) { // perhaps later I will add the posibility to raise
    if (state_ != State::TOEP) {
        std::cerr << "cannot play toep because the game is not in toep." << std::endl;
        return;
    }
    if (currentToeper_ != playerIndex) {
        std::cerr << "cannot play toep because Player " << playerIndex << " is not the current toeper." << std::endl;
        return;
    }
    if (!call) {
        for (int i = 0; i < players_[playerIndex].getHand().size(); ++i) {
            Card& card = players_[playerIndex].getHand().getCard(i);
            card.done();
        }
        players_[playerIndex].addScore(wager_);
        players_[playerIndex].fold();
        activePlayers_--;
    }
    nextPlayer();
}

void Game::playCard(size_t playerIndex, size_t cardIndex) {
    if (currentPlayer_ != playerIndex) {
        std::cerr << "it is not Player " <<  playerIndex << "'s turn." << std::endl;
        return;
    }
    int playerNo = currentPlayer_ - trickStarter_ % size();
    if (playerNo < 0) playerNo += size();
    if (playerNo < cardsInPlay_.size()) {
        std::cerr << "Player " << playerIndex << " already played their card." << std::endl;
        return;
    }

    Hand& hand = players_[currentPlayer_].getHand();
    if (getLeadingSuit()) {
        Card::Suit leadingSuit = *getLeadingSuit();
        Card::Suit mySuit = hand.getCard(cardIndex).suit();
        if (mySuit != leadingSuit && hand.canPlay(leadingSuit)) {
            notifyAI();
            return;
        }
    }
    players_[playerIndex].playCard(cardIndex);
    cardsInPlay_.push_back(&hand.getCard(cardIndex));
    nextPlayer();
}

void Game::evaluateTrick() {
    Card::Suit suit = cardsInPlay_[0]->suit();
    size_t maxIndex = 0;
    for (size_t i = 0; i < cardsInPlay_.size(); ++i) {
        if (cardsInPlay_[i]->suit() == suit && cardsInPlay_[i]->value() > cardsInPlay_[maxIndex] -> value()) {
            maxIndex = i;
        }
    }
    for (size_t i = 0; i < cardsInPlay_.size(); ++i) {
        if (i == maxIndex) cardsInPlay_[i]->win();
        else cardsInPlay_[i]->lose();
    }

    // calculate the winning player index
    int i = maxIndex;
    int pos = trickStarter_;
    while (i > 0) {
        if (players_[pos].isParticipating()) i--;
        pos = (pos + 1) % players_.size();
    }
    while (!players_[pos].isParticipating()) pos = (pos + 1) % players_.size();
    trickStarter_ = pos;

    currentPlayer_ = trickStarter_;
    trick_++;
    if (trick_ == players_.size()) {
        for (size_t i = 0; i < players_.size(); ++i) {
            if (i != trickStarter_ && players_[i].isParticipating()) players_[i].addScore(wager_);
        }
    }
}

void Game::closeRound() {
    roundStarter_ = (roundStarter_ + 1) % players_.size();
    trickStarter_ = roundStarter_;
    currentPlayer_ = roundStarter_;
}

void Game::evaluateToep() {
    wager_++;
    state_ = State::PLAY;
}

std::optional<Card::Suit> Game::getLeadingSuit() {
    if (cardsInPlay_.size() == 0) return std::nullopt;
    else return cardsInPlay_[0]->suit();
}

int Game::getWinner() {
    return winner_;
}

size_t Game::getWager() {
    return wager_;
}


std::vector<Card*> Game::getCardsInPlay() const {
    return cardsInPlay_;
}

void Game::unPause() {
    if (std::chrono::steady_clock::now() - lastActionTime >= std::chrono::milliseconds(100)) paused = false;
}

bool Game::isStartingToeper(size_t playerIndex) {
    return toepStarter_ == playerIndex;
}
