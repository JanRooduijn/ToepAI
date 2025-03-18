#include "../include/server.h"
#include <iostream>

Server::Server(boost::asio::io_context& io_context, const unsigned short port) : io_context_(io_context), freshConnectionID_(0),
        acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)), gameHandler_(GameHandler(*this)) {
    listen();
}

void Server::listen() {
    auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            std::cout << "Client connected from " << socket->remote_endpoint() << std::endl;
            handleClient(socket);
        }
        listen();
    });
}

void Server::handleClient(std::shared_ptr<tcp::socket> socket) {
    auto freshConnectionID = freshConnectionID_;
    freshConnectionID_++;
    connections_[freshConnectionID] = socket;
    receiveData(freshConnectionID);
}

// This function contains a buffer issue
void Server::receiveData(ConnectionID connectionID) {
    auto socket = connections_[connectionID];
    if (!socket || !socket->is_open()) {
        std::cerr << "Socket for connection " << connectionID << " is not valid." << std::endl;
        connections_.erase(connectionID);
        return;
    }

    auto buffer = std::make_shared<boost::asio::streambuf>();
    async_read_until(
    *socket,
    *buffer,
    '\n',
    [this, connectionID, socket, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            if (ec == boost::asio::error::eof) {
                std::cout << "Connection " << connectionID << " closed by peer.\n";
            } else {
                std::cerr << "Error reading data on connection "
                          << connectionID << ": " << ec.message() << "\n";
            }
            gameHandler_.removePlayer(connectionID);
            connections_.erase(connectionID);
            return;
        }

        std::istream is(buffer.get());
        std::string line;
        if (std::getline(is, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            processLine(connectionID, line);
        }

        receiveData(connectionID);
    });
}

void Server::processLine(ConnectionID connectionID, const std::string& line) {
    auto pos = line.find(' ');
    if (pos == std::string::npos) {
        std::cerr << "Invalid data sent by " << connectionID << "." << std::endl;
        connections_.erase(connectionID);
        return;
    }
    auto action = line.substr(0, pos);

    auto value = line.substr(pos);
    int val;
    try {
        val = std::stoi(value);
    }
    catch (...) {
        std::cerr << "Invalid data sent by " << connectionID << "." << std::endl;
        connections_.erase(connectionID);
        return;
    }

    gameHandler_.parseAction(connectionID, action, val);
}

void Server::sendLine(ConnectionID connectionID, const std::string& line) {
    auto it = connections_.find(connectionID);
    if (it == connections_.end()) {
        std::cerr << "Connection ID " << connectionID << " not found.\n";
        return;
    }

    auto socket = it->second;
    if (!socket || !socket->is_open()) {
        std::cerr << "Socket for connection " << connectionID << " is not valid.\n";
        connections_.erase(it);
        return;
    }

    std::string message = line + "\n";
    auto buffer = std::make_shared<std::string>(std::move(message));

    async_write(*socket, boost::asio::buffer(*buffer),
        [this, connectionID, buffer](boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending data to connection "
                          << connectionID << ": " << ec.message() << std::endl;
                auto it = connections_.find(connectionID);
                if (it != connections_.end()) {
                    boost::system::error_code ignored_ec;
                    it->second->shutdown(tcp::socket::shutdown_both, ignored_ec);
                    it->second->close(ignored_ec);
                    connections_.erase(it);
                }
            }
        }
    );
}

