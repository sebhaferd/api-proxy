#pragma once
#include "router.hpp"
#include "sql-logger.hpp"
#include "cache.hpp"



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
    ResponseCache cache;
    void return_recent_logs(int client_fd);
    void error404(int client_fd);
};