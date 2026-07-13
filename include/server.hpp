#pragma once
#include "router.hpp"
#include "sql-logger.hpp"
#include "cache.hpp"
#include <openssl/ssl.h>




class Server{
public:
    explicit Server(int port);
    ~Server();
    void start();

private:
    int port;
    int server_fd;
    SSL_CTX *ssl_context;

    bool setup_socket();
    bool setup_tls(
        const std::string& certificate_path,
        const std::string& private_key_path)
    void accept_loop();
    void handle_client(int cliend_fd);
    Router router;
    SqlLogger logger;
    ResponseCache cache;
    void return_recent_logs(int client_fd);
    void error404(int client_fd);
};