#include "../include/connectionhandler.h"

ConnectionHandler::ConnectionHandler(Game& game, std::mutex& gameMutex) :
client_(HOST, PORT, *this), gameMutex_(gameMutex), game_(game) {
    sendAction("HELLO", 0);
}

void ConnectionHandler::sendAction(const std::string& action, int val) {
    client_.sendLine(action + ' ' + std::to_string(val));
}

void ConnectionHandler::parseAction(const std::string& action, int val) {
    std::cout << "Received " << action << " " << val << " from server." << std::endl;

    if (action == "HELLO") { assignPlayer(val); }
    else if (action == "GAME") { loadGameData(val); }
    else if (action == "PLAYER") { loadPlayerTally(val); }
    else if (action == "CARD") { loadCard(val); }
    else if (action == "TRICK") { loadTrick(val); }
    else std::cerr << "Received invalid action: " << action << " from server. " << std::endl;
}

void ConnectionHandler::assignPlayer(PlayerIndex playerIndex) {
    playerIndex_ = playerIndex;
}

PlayerIndex ConnectionHandler::convertPlayer(PlayerIndex playerIndex) {
    int pClient = playerIndex_;
    int pServer = playerIndex;
    int ret = (pServer - pClient) % game_.size();
    if (ret < 0) ret += game_.size();
    return ret;
}

PlayerIndex ConnectionHandler::revertPlayer(PlayerIndex playerIndex) {
    return (playerIndex + playerIndex_) % game_.size();
}

void ConnectionHandler::loadGameData(int val) {
    std::lock_guard lock(gameMutex_);

    game_.setWager(decode(val));
    game_.setCurrentToeper(convertPlayer(decode(val)));
    game_.setToepStarter(convertPlayer(decode(val)));
    game_.setCurrentPlayer(convertPlayer(decode(val)));
    game_.setRoundStarter(convertPlayer(decode(val)));
    game_.setState(static_cast<Game::State>(decode(val)));
}

void ConnectionHandler::loadPlayerTally(int val) {
    std::lock_guard lock(gameMutex_);

    size_t score = decode(val);
    PlayerIndex playerIndex = convertPlayer(decode(val));

    game_.setScore(playerIndex, score);
}

void ConnectionHandler::loadCard(int val) {
    std::lock_guard lock(gameMutex_);

    size_t trickIndex = static_cast<unsigned char>(decode(val));
    bool won = (decode(val) == 1);
    Card::State state = static_cast<Card::State>(decode(val));
    Card::Suit suit = static_cast<Card::Suit>(decode(val));
    int value = decode(val) + 3;
    CardIndex cardIndex = decode(val);
    PlayerIndex playerIndex = convertPlayer(decode(val));

    Card card(value, suit, state, won, trickIndex);
    game_.setCard(playerIndex, cardIndex, card);
}

void ConnectionHandler::loadTrick(int val) {
    std::lock_guard lock(gameMutex_);

    Trick trick;
    while (val >= 10) {
        CardIndex cardIndex = decode(val);
        PlayerIndex playerIndex = convertPlayer(decode(val));
        trick.emplace_back(playerIndex, cardIndex, 0, 0);
    }

    game_.setCurrentTrick(trick);
    game_.setTrickNo(decode(val));
}


bool ConnectionHandler::connected() {
    return client_.connected();
}

void ConnectionHandler::sendMove(const Move& move) {
    int val = encodeDigits({static_cast<char>(revertPlayer(move.playerIndex)), static_cast<char>(move.cardIndex)
        , static_cast<char>(move.fold), static_cast<char>(move.raise)});
    sendAction("MOVE", val);
}

void ConnectionHandler::sendToep() {
    int val = 0;
    sendAction("TOEP", val);
}

void ConnectionHandler::sendFoldCall(bool call) {
    int val = 0;
    if (call) val++;
    sendAction("FOLDCALL", val);
}


int ConnectionHandler::encodeDigits(std::initializer_list<char> digits) {
    int ret = 0;
    for (char digit : digits) {
        if (digit < 0 || digit > 8)
            throw std::range_error(std::format("Cannot encode digit {}", digit));

        digit += 1;
        ret += digit;
        ret *= 10;
    }

    ret /= 10;
    return ret;
}


char ConnectionHandler::decode(int& val) {
    char ret = val % 10 - 1;
    val /= 10;
    return ret;
}
