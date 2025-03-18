#include "../include/gamehandler.h"
#include "../include/server.h"

GameHandler::GameHandler(Server &server) : server_(server), freshGameID_(0), running_(true) {
    updateThread_ = std::thread(&GameHandler::updateLoop, this);
}


void GameHandler::addPlayer(ConnectionID connectionID) {
    std::lock_guard lock(gameMutex_);

    if (getGame_.contains(connectionID)) {
        std::cerr << "ConnectionID " + std::to_string(connectionID) + " has already been assigned a game (namely with gameID " + std::to_string(getGame_[connectionID]) + ")" << std::endl;
        return;
    }

    int aiPlayers = 0;
    GameID gameID = 0;

    for (auto& [gID, game] : games_) {
        if (game.aiPlayers() == 0) continue;
        if (game.aiPlayers() < aiPlayers || aiPlayers == 0) {
            aiPlayers = game.aiPlayers();
            gameID = gID;
        }
    }

    // No suitable game found, so we create a new Game
    if (aiPlayers == 0) {
        gameID = freshGameID_;
        freshGameID_++;
        games_.try_emplace(gameID, 4, 4, true);
        getPlayer_[connectionID] = 0;
        getGame_[connectionID] = gameID;
        getConnectionIDs_.try_emplace(gameID, 1, connectionID);
    }

    // Add player to existing game
    else {
        auto it = games_.find(gameID);
        if (it == games_.end()) {
            std::cout << "Error: no game found";
            return;
        }

        Game& game = it->second;
        for (PlayerIndex i = 0; i < game.size(); ++i) {
            if (game.getPlayer(i).isAI()) {
                getPlayer_[connectionID] = i;
                game.makeHuman(i);
                break;
            }
        }
        getGame_[connectionID] = gameID;
        getConnectionIDs_[gameID].push_back(connectionID);
    }
    sendAction(connectionID, "HELLO", getPlayer_[connectionID]);
    sendGameData(connectionID);
    sendPlayerTallies(connectionID);
    sendCards(connectionID);
    sendTrick(connectionID);
}

void GameHandler::removePlayer(ConnectionID connectionID) {
    std::lock_guard lock(gameMutex_);

    auto it = getGame_.find(connectionID);
    if (it == getGame_.end()) {
        std::cerr << "No GameID found for ConnectionID " << connectionID << "." << std::endl;
        return;
    }

    GameID gameID = it->second;
    auto itg = games_.find(gameID);
    if (itg == games_.end()) {
        std::cout << "Error: no game found";
        return;
    }
    Game& game = itg->second;

    PlayerIndex playerIndex = getPlayer_[connectionID];
    game.makeAI(playerIndex);

    getPlayer_.erase(connectionID);
    getGame_.erase(connectionID);
    {
        auto itc = std::find(getConnectionIDs_[gameID].begin(), getConnectionIDs_[gameID].end(), connectionID);
        if (itc == getConnectionIDs_[gameID].end()) std::cerr << "ConnectionID " << connectionID << " not found for GameID " << gameID << std::endl;
        else getConnectionIDs_[gameID].erase(itc);
    }
    if (game.aiPlayers() == game.size()) games_.erase(gameID);
}


void GameHandler::parseAction(ConnectionID connectionID, const std::string& action, int val) {
    if (action == "HELLO")
        addPlayer(connectionID);
    else if (action == "MOVE")
        parseMove(connectionID, val);
    else if (action == "TOEP")
        parseToep(connectionID, val);
    else if (action == "FOLDCALL")
        parseFoldCall(connectionID,  val);
    else
        std::cerr << "Received invalid action: " << action << " from connectionID " << connectionID << "." << std::endl;
}

void GameHandler::parseMove(ConnectionID connectionID, int val) {
    Move move;
    move.raise = decode(val);
    move.fold = decode(val);
    move.cardIndex = decode(val);
    move.playerIndex = decode(val);

    try {
        GameID gameID = getGame_.at(connectionID);
        Game& game = games_.at(gameID);
        game.playCard(move.playerIndex, move.cardIndex);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::parseToep(ConnectionID connectionID, int val) {
    try {
        GameID gameID = getGame_.at(connectionID);
        Game& game = games_.at(gameID);
        PlayerIndex playerIndex = getPlayer_.at(connectionID);
        game.toep(playerIndex);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game or player corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::parseFoldCall(ConnectionID connectionID, int val) {
    try {
        GameID gameID = getGame_.at(connectionID);
        Game& game = games_.at(gameID);
        PlayerIndex playerIndex = getPlayer_.at(connectionID);
        bool call = (val == 1);
        game.playToep(playerIndex, call);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game or player corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::sendAction(ConnectionID connectionID, const std::string& action, int val) {
    server_.sendLine(connectionID, action + ' ' + std::to_string(val));
}


void GameHandler::updateLoop() {
    while (running_) {
        {
            for (auto& [gameID, game] : games_) {
                if (game.update()) {
                    std::lock_guard lock(gameMutex_);
                    for (auto connectionID : getConnectionIDs_[gameID]) {
                        sendGameData(connectionID);
                        sendPlayerTallies(connectionID);
                        sendCards(connectionID);
                        sendTrick(connectionID);
                    }
                }
            }
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void GameHandler::sendGameData(ConnectionID connectionID) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);

        int val = encodeDigits({static_cast<char>(game.getState()), static_cast<char>(game.getRoundStarter()),
            static_cast<char>(game.getCurrentPlayer()), static_cast<char>(game.getToepStarter()),
            static_cast<char>(game.getCurrentToeper()), static_cast<char>(game.getWager())});
        sendAction(connectionID, "GAME", val);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::sendPlayerTally(ConnectionID connectionID, PlayerIndex playerIndex) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);

        int val = encodeDigits({static_cast<char>(playerIndex), static_cast<char>(game.getPlayer(playerIndex).score())});
        sendAction(connectionID, "PLAYER", val);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::sendPlayerTallies(ConnectionID connectionID) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);

        for (PlayerIndex i = 0; i < game.size(); ++i) sendPlayerTally(connectionID, i);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}


void GameHandler::sendCard(ConnectionID connectionID, PlayerIndex playerIndex, CardIndex cardIndex) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);
        PlayerIndex connectionPlayerIndex = getPlayer_.at(connectionID);

        Card card = game.getPlayer(playerIndex).getHand().getCard(cardIndex);
        if (playerIndex != connectionPlayerIndex && card.state() == Card::State::INIT) {
            card.setValue(10);
            card.setSuit(Card::SPADES);
        }

        int val = encodeDigits({static_cast<char>(playerIndex), static_cast<char>(cardIndex), static_cast<char>(card.value() - 3),
            static_cast<char>(card.suit()), static_cast<char>(card.state()), static_cast<char>(card.won()), static_cast<char>(card.getTrickIndex())});
        sendAction(connectionID, "CARD", val);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game or player corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::sendCards(ConnectionID connectionID) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);

        PlayerIndex playerIndex = getPlayer_.at(connectionID);
        for (PlayerIndex i = 0; i < game.size(); i++) {
            const Hand& hand = game.getPlayer(i).getHand();
            for (CardIndex j = 0; j < hand.size(); j++) {
                const Card& card = hand.getCard(j);
                sendCard(connectionID, i, j);
            }
        }

    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game or player corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

void GameHandler::sendTrick(ConnectionID connectionID) {
    try {
        GameID gameID = getGame_.at(connectionID);
        const Game& game = games_.at(gameID);
        const Trick& trick = game.getCurrentTrick();

        int val = game.getTrickNo() + 1;
        for (int i = trick.size() - 1; i >= 0; --i) {
            const Move& move = trick[i];
            val *= 10; val += move.playerIndex + 1;
            val *= 10; val += move.cardIndex + 1;
        }

        sendAction(connectionID, "TRICK", val);
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Failed to get game corresponding to ConnectionID " << connectionID << " (" << e.what() << ")." << std::endl;
    }
}

int GameHandler::encodeDigits(std::initializer_list<char> digits) {
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

char GameHandler::decode(int& val) {
    char ret = val % 10 - 1;
    val /= 10;
    return ret;
}
