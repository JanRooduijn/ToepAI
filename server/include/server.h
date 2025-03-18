#pragma once
#include "gamehandler.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, const unsigned short port);
    void sendLine(ConnectionID connectionID, const std::string& line);

private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::unordered_map<ConnectionID, std::shared_ptr<tcp::socket>> connections_;
    ConnectionID freshConnectionID_;
    GameHandler gameHandler_;

    void listen();
    void handleClient(std::shared_ptr<tcp::socket> socket);
    void receiveData(ConnectionID connectionID);
    void processLine(ConnectionID connectionID, const std::string& line);
};



