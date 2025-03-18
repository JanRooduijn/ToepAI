#pragma once
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class ConnectionHandler;

class Client {
public:
    Client(const std::string& host, const unsigned short port, ConnectionHandler& connectionHandler);
    ~Client();
    void sendLine(const std::string& line);
    bool connected();

private:
    boost::asio::io_context io_context_;
    boost::asio::streambuf buffer_;
    tcp::socket socket_;
    std::thread io_thread_;
    ConnectionHandler& connectionHandler_;

    void listen();
    void receiveData();
    void processLine(const std::string& line);
    void closeConnection();
    bool connected_;
};
