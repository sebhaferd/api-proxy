#pragma once
#include "router.hpp"
#include "sql-logger.hpp"



class Server{
public:
    explicit Server(int port);
    void start();

private:
    int port;
    int server_fd;

    bool setup_socket();
    void accept_loop();
    void handle_client(int cliend_fd);
    Router router;
    SqlLogger logger;
};