#include "../include/gamehandler.h"
#include "../include/server.h"

int main()
{
    boost::asio::io_context io_context;
    Server server(io_context, 55215);
    io_context.run();
    return 0;
}
