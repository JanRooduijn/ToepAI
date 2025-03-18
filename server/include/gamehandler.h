#pragma once
#include <thread>
#include "../../shared/include/game.h"

class Server;
using ConnectionID = std::size_t;
using GameID = std::size_t;

class GameHandler {
public:
    GameHandler(Server& server);
    void addPlayer(ConnectionID connectionID);
    void removePlayer(ConnectionID connectionID);
    void parseAction(ConnectionID connectionID, const std::string& action, int val);

private:
    void parseMove(ConnectionID connectionID, int val);
    void parseToep(ConnectionID connectionID, int val);
    void parseFoldCall(ConnectionID connectionID, int val);
    void sendAction(ConnectionID connectionID, const std::string& action, int val);
    void sendGameData(ConnectionID connectionID);
    void sendPlayerTally(ConnectionID connectionID, PlayerIndex playerIndex);
    void sendPlayerTallies(ConnectionID connectionID);
    void sendCard(ConnectionID connectionID, PlayerIndex playerIndex, CardIndex cardIndex);
    void sendCards(ConnectionID connectionID);
    void sendTrick(ConnectionID connectionID);

    Server& server_;
    GameID freshGameID_;
    std::unordered_map<GameID, Game> games_;
    std::unordered_map<ConnectionID, PlayerIndex> getPlayer_;
    std::unordered_map<ConnectionID, GameID> getGame_;
    std::unordered_map<GameID, std::vector<int>> getConnectionIDs_;
    void updateLoop();
    std::mutex gameMutex_;
    bool running_;
    std::thread updateThread_;
    static int encodeDigits(std::initializer_list<char>);
    static char decode(int& val);
};



