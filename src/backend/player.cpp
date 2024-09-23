#include "../../include/backend/player.h"

Player::Player(Hand& hand, bool isAI) : hand_(hand), score_(0),  isAI_(isAI) {}
Hand& Player::getHand() { return hand_; }
void Player::setHand(const Hand& hand) { hand_ = hand; }
size_t Player::score() const { return score_; }
void Player::addScore(size_t n) { score_ += n; }
bool Player::isAI() const { return isAI_; }
void Player::playCard(size_t cardIndex) { hand_.playCard(cardIndex); }
bool Player::isParticipating() { return participating_; }
void Player::participate() { participating_ = true; }
void Player::fold() { participating_ = false; }

