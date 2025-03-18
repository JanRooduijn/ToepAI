#include "../include/client.h"
#include "../include/connectionHandler.h"
#include <iostream>


Client::Client(const std::string& host, const unsigned short port, ConnectionHandler& connectionHandler) :
socket_(io_context_), connectionHandler_(connectionHandler) {
    try {
        tcp::resolver resolver(io_context_);
        boost::asio::connect(socket_, resolver.resolve(host, std::to_string(port)));
        std::cout << "Connected to server at " << host << ":" << port << std::endl;
        receiveData();

        io_thread_ = std::thread([this]() {
            connected_ = true;
            try {
                    io_context_.run();
                } catch (const std::exception& e) {
                    std::cerr << "I/O thread error: " << e.what() << std::endl;
                }
            });
        } catch (const std::exception& e) {
            std::cerr << "Client connection failed: " << e.what() << std::endl;
            connected_ = false;
        }
}

Client::~Client() {
    closeConnection();
    io_context_.stop();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}


void Client::sendLine(const std::string& line) {
    std::string message = line + "\n";
    auto buffer = std::make_shared<std::string>(std::move(message));

    async_write(socket_, boost::asio::buffer(*buffer),
        [this, buffer](boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error sending data to server: " << ec.message() << std::endl;
                closeConnection();
            }
        }
    );
}

void Client::receiveData() {
    async_read_until(socket_, buffer_, '\n',
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec) {
                if (ec == boost::asio::error::eof) {
                    std::cout << "Disconnected from server." << std::endl;
                } else {
                    std::cerr << "Received error: " << ec.message() << std::endl;
                }
                closeConnection();
                return;
            }

            std::istream is(&buffer_);
            std::string line;
            std::getline(is, line);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            processLine(line);
            receiveData();
        });
}

void Client::processLine(const std::string& line) {
    auto pos = line.find(' ');
    if (pos == std::string::npos) {
        std::cerr << "Invalid data sent by server." << std::endl;
        closeConnection();
        return;
    }
    auto action = line.substr(0, pos);

    auto value = line.substr(pos);
    int val;
    try {
        val = std::stoi(value);
    }
    catch (...) {
        std::cerr << "Invalid data sent by server." << std::endl;
        closeConnection();
        return;
    }

    connectionHandler_.parseAction(action, val);
}

void Client::closeConnection() {
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    connected_ = false;
    std::cout << "Closed connection to server." << std::endl;
}

bool Client::connected() { return connected_; }