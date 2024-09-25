#include "../include/backend/game.h"

#include <iostream>
#include <random>
#include <thread>
#include <__random/random_device.h>

Game::Game(const size_t& playerSize, const size_t& handSize) :
    handSize_(handSize), state_(Game::State::INIT),
    winner_(-1), trickNo_(0), first_(true), activePlayers_(0) {
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
    roundStarter_ = &players_[dis(g)];
    currentPlayer_ = roundStarter_;

    lastActionTime = std::chrono::steady_clock::now() + pauseTime;
    paused = true;
}

Game::State Game::getState() const { return state_; }

void Game::update() {
    if (state_ != State::TRICK_DONE) {
        if (!currentPlayer_->isParticipating()) unPause();
        if (std::chrono::steady_clock::now() - lastActionTime >= pauseTime) unPause();
    }
    if (paused) return;

    switch (state_) {
        case State::INIT:
            state_ = State::PLAY;

        case State::PLAY:
            if (currentTrick_.size() > 0 && currentPlayer_ == currentTrick_.getStartingMove()->getPlayer()) {
                evaluateTrick();
                state_ = State::TRICK_DONE;
            }
            else if (activePlayers_ == 1) state_ = State::TRICK_DONE;
            else if (!currentPlayer_->isParticipating()) nextPlayer();
            else notifyAI();
            break;

        case State::TOEP:
            if (currentToeper_ == toepStarter_) {
                evaluateToep();
            }
            else if (!currentToeper_->isParticipating()) nextPlayer();
            else notifyAI();
            break;

        case State::TRICK_DONE:
            // Mark all cards in the current trick as done
            for (Card* card : currentTrick_.getCards()) card->done();

            // Add the currentTrick to the history
            tricks_.push_back(currentTrick_);

            if (activePlayers_ == 1 || tricks_.size() == players_.size()) {
                closeRound();
                trickNo_ = 0;
                wager_ = 1;
                tricks_.clear();
                dealHands();
            }

            // Create a new Trick
            currentTrick_ = Trick();

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

Player* Game::getPlayer(size_t index) {
    if (index > players_.size() - 1) throw std::invalid_argument("Invalid index");
    return &players_[index];
}
size_t Game::size() const { return players_.size(); }
size_t Game::getHandSize() const { return handSize_; }
Player* Game::getCurrentPlayer() const { return currentPlayer_; }

void Game::nextPlayer() {
    size_t index;
    switch (state_) {
        case State::PLAY:
            index = getPlayerIndex(currentPlayer_);
            index = (index + 1) % players_.size();
            currentPlayer_ = &players_[index];
            break;

        case State::TOEP:
            index = getPlayerIndex(currentToeper_);
            index = (index + 1) % players_.size();
            currentToeper_ = &players_[index];
            break;

        default:
            std::cerr << "Cannot switch to next player, because game is not in play or in toep." << std::endl;
    }

    lastActionTime = std::chrono::steady_clock::now();
    paused = true;
}

void Game::notifyAI() {
    switch (state_) {
        case State::PLAY:
            if (currentPlayer_->isAI()) AI::getInstance().play(this, currentPlayer_);
            break;
        case State::TOEP:
            if (currentToeper_->isAI()) AI::getInstance().toep(this, currentToeper_);
            break;
        default:
            break;
    }
}

void Game::toep(Player* player) {
    if (state_ != State::PLAY) {
        std::cerr << "cannot toep because the game is not in play." << std::endl;
        return;
    }
    state_ = State::TOEP;
    toepStarter_= player;
    currentToeper_ = player;
    nextPlayer();
}

void Game::playToep(Player* player, bool call) { // perhaps later I will add the possibility to raise
    if (state_ != State::TOEP) {
        std::cerr << "cannot play toep because the game is not in toep." << std::endl;
        return;
    }
    if (currentToeper_ != player) {
        std::cerr << "cannot play toep because it is not Player" + std::to_string(getPlayerIndex(player)) + "'s turn." << std::endl;
        return;
    }
    if (!call) {
        Hand& hand = player->getHand();
        for (int i = 0; i < hand.size(); ++i) {
            Card& card = hand.getCard(i);
            card.done();
        }
        player->addScore(wager_);
        player->fold();
        currentTrick_.setFold(player, wager_);
        activePlayers_--;
    }
    nextPlayer();
}

void Game::playCard(Player* player, size_t cardIndex) {
    if (currentPlayer_ != player) {
        std::cerr << "it is not Player " <<  std::to_string(getPlayerIndex(player)) << "'s turn." << std::endl;
        return;
    }
    int playerNo = 0;
    if (currentTrick_.size() > 0) playerNo = getPlayerIndex(currentPlayer_) - getPlayerIndex(currentTrick_.getStartingMove()->getPlayer()) % size();
    if (playerNo < 0) playerNo += size();
    if (playerNo < currentTrick_.size()) {
        std::cerr << "Player " << std::to_string(getPlayerIndex(player)) << " already played their card." << std::endl;
        return;
    }

    Hand& hand = currentPlayer_->getHand();
    if (getLeadingSuit()) {
        Card::Suit leadingSuit = *getLeadingSuit();
        Card::Suit mySuit = hand.getCard(cardIndex).suit();
        if (mySuit != leadingSuit && hand.canPlay(leadingSuit)) {
            notifyAI();
            return;
        }
    }
    player->playCard(cardIndex);
    Move move(player, &hand.getCard(cardIndex));
    currentTrick_.addMove(move);
    nextPlayer();
}

void Game::evaluateTrick() {
    Move* startingMove = currentTrick_.getStartingMove();
    if (currentTrick_.size() == 0) {
        std::cerr << "evaluateTrick() called on empty trick";
        return;
    }

    Card::Suit suit = startingMove->getCard()->suit();
    size_t maxIndex = 0;
    auto cards = currentTrick_.getCards();
    for (size_t i = 0; i < cards.size(); ++i) {
        if (cards[i]->suit() == suit && cards[i]->value() > cards[maxIndex] -> value()) {
            maxIndex = i;
        }
    }
    for (size_t i = 0; i < cards.size(); ++i) {
        if (i == maxIndex) cards[i]->win();
        else cards[i]->lose();
    }

    // calculate the winning player index
    int i = maxIndex;
    int pos = getPlayerIndex(startingMove->getPlayer());
    while (i > 0) {
        if (players_[pos].isParticipating()) i--;
        pos = (pos + 1) % players_.size();
    }
    while (!players_[pos].isParticipating()) pos = (pos + 1) % players_.size();

    currentPlayer_ = &players_[pos];;
    trickNo_++;
    if (trickNo_ == players_.size()) {
        for (size_t i = 0; i < players_.size(); ++i) {
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
    currentTrick_.setRaise(currentToeper_, wager_);
    state_ = State::PLAY;
}

std::optional<Card::Suit> Game::getLeadingSuit() {
    if (currentTrick_.size() == 0) return std::nullopt;
    else return currentTrick_.getStartingMove()->getCard()->suit();
}

int Game::getWinner() {
    return winner_;
}

size_t Game::getWager() {
    return wager_;
}


std::vector<Card*> Game::getCardsInPlay() const {
    return currentTrick_.getCards();
}

void Game::unPause() {
    if (std::chrono::steady_clock::now() - lastActionTime >= std::chrono::milliseconds(100)) paused = false;
}

bool Game::isStartingToeper(Player* player) {
    return toepStarter_ == player;
}

const Trick& Game::getCurrentTrick() const {
    return currentTrick_;
}

const size_t Game::getTrickNo() const {
    return trickNo_;
}


int Game::getPlayerIndex(Player* player) {
    for (size_t i = 0; i < players_.size(); ++i) {
        if (&players_[i] == player) return i;
    }
    std::cerr << "Player index not found";
    return -1;
}

