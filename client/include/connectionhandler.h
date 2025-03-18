#pragma once
#include <iostream>
#include "client.h"
#include "../../shared/include/game.h"

class ConnectionHandler {
public:
    ConnectionHandler(Game& game, std::mutex& gameMutex);
    void sendAction(const std::string& action, int val);
    void parseAction(const std::string& action, int val);
    void assignPlayer(PlayerIndex playerIndex);
    PlayerIndex convertPlayer(PlayerIndex playerIndex);
    PlayerIndex revertPlayer(PlayerIndex playerIndex);
    bool connected();
    void loadGameData(int val);
    void loadPlayerTally(int val);
    void loadCard(int val);
    void loadTrick(int val);
    void sendMove(const Move &move);
    void sendToep();
    void sendFoldCall(bool call);

private:
    static constexpr const char* HOST = "127.0.0.1";
    static constexpr unsigned short PORT = 55215;

    Game& game_;
    std::mutex& gameMutex_;
    Client client_;
    PlayerIndex playerIndex_;

    static int encodeDigits(std::initializer_list<char> digits);
    static char decode(int& number);
};



