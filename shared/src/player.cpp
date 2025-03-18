#include "../include/player.h"

Player::Player(Hand& hand, bool isAI) : hand_(hand), score_(0),  isAI_(isAI), wager_(1) {}
const Hand& Player::getHand() const { return hand_; }
void Player::setHand(Hand hand) { hand_ = hand; }
size_t Player::score() const { return score_; }
void Player::addScore(size_t score) { score_ += score; }
void Player::setScore(size_t score) { score_ = score; }
bool Player::isAI() const { return isAI_; }
void Player::playCard(CardIndex cardIndex, size_t trickIndex) { hand_.playCard(cardIndex, trickIndex); }
void Player::setCard(CardIndex cardIndex, Card card) { hand_.setCard(cardIndex, card); }
void Player::win(CardIndex cardIndex) { hand_.win(cardIndex); }
void Player::lose(CardIndex cardIndex) { hand_.lose(cardIndex); }
void Player::done(CardIndex cardIndex) { hand_.done(cardIndex); }
bool Player::isParticipating() { return participating_; }
void Player::participate() { participating_ = true; }
void Player::fold() { participating_ = false; }
void Player::makeAI() { isAI_ = true; }
void Player::makeHuman() { isAI_ = false; }



