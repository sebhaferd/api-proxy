#pragma once
#include "router.hpp"
#include "sql-logger.hpp"
#include "http-request.hpp"
#include "rate-limiter.hpp"
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
    SSL_CTX *forward_context;

    bool setup_socket();
    bool setup_tls(
        const std::string& certificate_path,
        const std::string& private_key_path);

    bool setup_forward_ssl();
    void accept_loop();
    void handle_client(int cliend_fd,  const std::string& client_ip);
    std::string forward_to_server(const RouteTarget& target, 
        const std::string& forward_path,
        HttpRequest& req);
    Router router;
    SqlLogger logger;
    ResponseCache cache;
    RateLimiter limiter;
    void return_recent_logs(SSL* client_ssl);
    void error404(SSL* client_ssl);
};