#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <chrono>
#include <string>
#include <sstream>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../../include/rate-limiter.hpp"
#include "../../include/sql-logger.hpp"
#include "../../include/server.hpp"
#include "../../include/http-request.hpp"
#include "../../include/router.hpp"
#include "../../include/thread-pool.hpp"


Server::Server(int port)
    : port(port), server_fd(-1), logger(), 
    cache(100), ssl_context(nullptr), 
    forward_context(nullptr), limiter(10, 2)
    {
    }

void Server::start(){
    if (!setup_socket()){
        std::cerr<<"Server setup failed"<<std::endl;
        return;
    }
    setup_tls("certs/server-cert.pem",
    "certs/server-key.pem");
    setup_forward_ssl();
    accept_loop();
}

Server::~Server(){
    if (ssl_context != nullptr){
        SSL_CTX_free(ssl_context);
    }
    if (forward_context != nullptr){
        SSL_CTX_free(forward_context);
    }
    if (server_fd != -1){
        close(server_fd);
    }
}

bool Server::setup_socket(){
    //get socket file descriptor
    server_fd = socket(AF_INET,SOCK_STREAM, 0);
    if (server_fd == -1){
        std::cerr<<"Failed to create socket \n";
        return false;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    //bind socket
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0){
        std::cerr<<"Bind failed\n";
        return false;
    }
    //listen for connections on port
    if (listen(server_fd, 10) < 0){
        std::cerr<<"Listen failed\n";
        return false;
    }
    std::cout<<"Listening on port"<< port <<std::endl;
    return true;
}

bool Server::setup_tls(
        const std::string& certificate_path,
        const std::string& private_key_path){
    ssl_context = SSL_CTX_new(TLS_server_method());

    //import certificate and private key to ssl ctx from input files
    SSL_CTX_use_certificate_file(ssl_context, certificate_path.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ssl_context, private_key_path.c_str(),SSL_FILETYPE_PEM);

    //ensure tls is configured correctly with version, certificate and private key
    if (ssl_context == nullptr){
        std::cerr<<"Failed to start tls context";
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (
        SSL_CTX_set_min_proto_version(
            ssl_context,
            TLS1_2_VERSION
        ) != 1
    ) {
        std::cerr << "Failed to set minimum TLS version\n";
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (
        SSL_CTX_use_certificate_chain_file(
            ssl_context,
            certificate_path.c_str()
        ) != 1
    ) {
        std::cerr << "Failed to load certificate\n";
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (
        SSL_CTX_use_PrivateKey_file(
            ssl_context,
            private_key_path.c_str(),
            SSL_FILETYPE_PEM
        ) != 1
    ) {
        std::cerr << "Failed to load private key\n";
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_check_private_key(ssl_context) != 1) {
        std::cerr << "Certificate and private key do not match\n";
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;

}

//use SSL_write to send data to ssl destination
bool ssl_send(SSL *ssl, const std::string& data){
    std::size_t total_written = 0;
    while (total_written < data.size()){
        int written = SSL_write(ssl, data.data() + total_written, static_cast<int>(data.size()-total_written));
        if (written < 0){
            int ssl_error = SSL_get_error(ssl, written);
            std::cerr<<"Failed to write ssl data";

            return false;
        }
        total_written += static_cast<std::size_t>(written);
    }
    return true;
}

bool Server::setup_forward_ssl(){
    forward_context = SSL_CTX_new(TLS_client_method());
    //enable certificate verification
    SSL_CTX_set_verify(forward_context, SSL_VERIFY_PEER, nullptr);

    if (SSL_CTX_set_default_verify_paths(forward_context) != 1){
        std::cerr<<"failed to load OS CA certificates";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(forward_context);
        forward_context = nullptr;
        return false;
    }
    return true;
}
//forward http request to server
std::string Server::forward_to_server(const RouteTarget& target, 
        const std::string& forward_path,
        HttpRequest& req){
    //temp hardcoded destination
    std::string dest_host = target.host;
    std::string dest_port = target.port;
    //create socket for dest
    int forward_fd = socket(AF_INET,SOCK_STREAM, 0);
    if (forward_fd == -1){
        std::cerr<<"Failed to forward data \n";
        return "";
    }
    //create dest address
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *result = nullptr;
    int info = getaddrinfo(dest_host.c_str(), dest_port.c_str(), &hints, &result);

    if (info != 0){
        std::cerr<<"getaddrinfo failed"<<std::endl;
        close(forward_fd);
        return "";
    }
    //connect address with socket
    if (connect(forward_fd, result->ai_addr, result->ai_addrlen) < 0){
        std::cerr<<"connect failed"<<std::endl;
        freeaddrinfo(result);
        close(forward_fd);
        return "";
    }

    //generate ssl object to encrypt upstream requests
    SSL *forward_ssl = SSL_new(forward_context);
    if (forward_ssl == nullptr){
        std::cerr<<"failed to create forwarding ssl connection"<<std::endl;
        ERR_print_errors_fp(stderr);
        close(forward_fd);
        return "";
    }

    //attatch forward socket to ssl
    if (SSL_set_fd(forward_ssl, forward_fd) != 1){
        std::cerr<<"failed to attatch foward socket to ssl"<<std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(forward_ssl);
        close(forward_fd);
        return "";
    }
    SSL_set_tlsext_host_name(forward_ssl, target.host.c_str());  
    SSL_set1_host(forward_ssl, target.host.c_str());
    if (SSL_connect(forward_ssl) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(forward_ssl);
        close(forward_fd);
        return "";
    }
    
    std::ostringstream request;

    request << req.method << " " << forward_path << " HTTP/1.1\r\n";
    request << "Host: " << target.host << "\r\n";
    request << "Connection: close\r\n";

    for (const auto& [key, val] : req.headers) {
        if (key != "Host" && key != "Connection") {
            request << key << ": " << val << "\r\n";
        }
    }

    

    std::string request_str = request.str();

    std::cout << "FORWARDED REQUEST:\n";
    std::cout << request_str << std::endl;

    ssl_send(forward_ssl, request_str.c_str());

    request << "\r\n";

    //recieve response from example.com
    std::string response;
    char buffer[4096];
    ssize_t bytes_recieved;
    while ((bytes_recieved = SSL_read(forward_ssl, buffer, sizeof(buffer))) > 0){
        response.append(buffer, bytes_recieved);
    }

    //free addrinfo ptr and close socket
    freeaddrinfo(result);
    SSL_shutdown(forward_ssl);
    SSL_free(forward_ssl);
    close(forward_fd);
    return response;
    
}

int get_status_code(const std::string& response){
    std::istringstream stream(response);

    std::string http_version;
    int status_code;
    stream >> http_version >> status_code;
    if (stream.fail()){
        return 0; // couldnt parse response
    }
    return status_code;
}



void Server::return_recent_logs(SSL* client_ssl){
    std::string body = logger.get_recent_logs_json();
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-length: " + std::to_string(body.size()) + "\r\n"
        + "\r\n" + body;
    ssl_send(client_ssl, response.c_str());
    return;
}

void Server::error404(SSL* client_ssl){
    std::string body = "Route not found";
    std::string not_found = 
        "HTTP/1.1 404 not found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "\r\n"
        + body;
    
    ssl_send(client_ssl, not_found.c_str());
}


void Server::handle_client(int client_fd, const std::string& client_ip){    
    //Create ssl for client using server ssl context
    SSL* client_ssl = SSL_new(ssl_context);
    if (client_ssl == nullptr){
        std::cerr<<"Failed to create client ssl";
        ERR_print_errors_fp(stderr);
        close(client_fd);
        return;
    }
    //attatch client socket to created ssl
    if (SSL_set_fd(client_ssl, client_fd) != 1){
        std::cerr<<"Failed to attatch client socket to ssl";
        ERR_print_errors_fp(stderr);
        SSL_free(client_ssl);
        close(client_fd);
        return;
    }
    //perform tls handshake with client and check if completed before parsing http req
    int handshake_res = SSL_accept(client_ssl);
    if (handshake_res <= 0){
        int ssl_error = SSL_get_error(client_ssl, handshake_res);
        std::cerr<<"ssl handshake failed";
        ERR_print_errors_fp(stderr);
        SSL_free(client_ssl);
        close(client_fd);
    }

    //check rate limiter to allow client to request

    if (!limiter.allow(client_ip)){
        std::string body = "Rate limit exceeded";
        std::string response =
            "HTTP/1.1 429 Too Many Requests\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " +
            std::to_string(body.size()) +
            "\r\n"
            "Retry-After: 1\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;

        ssl_send(client_ssl, response);

        SSL_shutdown(client_ssl);
        SSL_free(client_ssl);
        close(client_fd);
        return;
    }
    //concurrency testing: output client socket ind and current thread
    std::cout << "client_fd=" << client_fd
          << " thread=" << std::this_thread::get_id()
          << std::endl;

    //set char buffer for input
    char buffer[4096] = {0};
    //start clock to measure latency
    auto start = std::chrono::steady_clock::now();
    //read bytes from client socket
    ssize_t bytes_read = SSL_read(client_ssl, (void*)buffer, sizeof(buffer)-1);
    if (bytes_read < 0){
        int ssl_error = SSL_get_error(client_ssl, bytes_read);
        std::cerr<< "recv failed";
        SSL_free(client_ssl);
        close(client_fd);
        return;
    }
    if (bytes_read == 0){
        std::cerr<< "Client closed connection";
        int ssl_error = SSL_get_error(client_ssl, bytes_read);
        SSL_free(client_ssl);
        close(client_fd);
        return;
    }

    //convert bytes to string
    std::string raw_data(buffer, bytes_read);
    HttpRequest req = HttpRequest::parse(raw_data);
    if (req.path == "/admin/logs"){
        return_recent_logs(client_ssl);
        return;
    }

    

    //Get destination for result from router vector
    Route route;
    if (!router.resolve(req.path, route)){
        error404(client_ssl);
        return;
    }
    bool headers_injected = false;

    //inject headers from route
    for (auto& [key, val] : route.injected_headers){
        req.headers[key] = val;
        headers_injected = true;
    }
    //update host information
    req.headers["Host"] = route.target.host;

    //Search caches for request

    //output request
    std::cout<<"Method:" << req.method<<"\n";
    std::cout<<"Path:"<<req.path<<"\n";
    std::cout<<"Version:"<<req.version<<std::endl;
    std::cout<<"Headers:"<<std::endl;


    for (const auto [key, val] : req.headers){
        std::cout<<key<<": "<<val<<std::endl;
    }
    
    //set forward path to substring after prefix of route target
    std::string forward_path = req.path.substr(route.prefix.size());
    if (forward_path.empty()){
        forward_path = "/";
    }
    
    //get response from backend target
    std::string cache_key;
    std::string response;
    bool hit = false;
    if (req.method == "GET"){
        cache_key = req.method + "|" +
                                req.headers["Host"] + "|" + 
                                req.path;
        hit = cache.get(cache_key, response);
    }

    if (!hit) {

        response = forward_to_server(route.target, forward_path, req);
        //no hit but valid response add to cache:
        if (req.method == "GET" && !response.empty() &&
                get_status_code(response) == 200){
            cache.put(cache_key, response, std::chrono::seconds(60));
        }
    }

    if (response.empty()) {
        std::string body = "Bad Gateway";
        std::string bad_gateway =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" +
            body;

        ssl_send(client_ssl, bad_gateway.c_str());
        SSL_shutdown(client_ssl);
        SSL_free(client_ssl);
        close(client_fd);
        return;
    }


    //parse for status code from application response, eg. 200 OK
    int status_code = get_status_code(response);
    //measure latency from client request intialization to response 
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    long latency = elapsed.count();

    //log request
    logger.log_request(req.method, req.path, route.target.host, 
                        forward_path, status_code, headers_injected, 
                        latency, response.size(), hit);

    ssl_send(client_ssl, response.c_str());
    SSL_shutdown(client_ssl);
    SSL_free(client_ssl);
    close(client_fd);

}


void Server::accept_loop(){
    //initialize threadpool to handle clients concurrently
    ThreadPool pool(4);
    //wait for client to connect to port annd accept tcp connection
    while (true){
        sockaddr_in client_address{};
        socklen_t address_size = sizeof(client_address);
        int client_fd = accept(server_fd, (sockaddr*)&client_address, &address_size);
        if (client_fd < 0){
            std::cerr<<"Accept failed\n";
            continue;
        }
        //get client ip address for rate limiting
        char ip_buffer[INET_ADDRSTRLEN];
        const char* ip_result = inet_ntop(
            AF_INET,
            &client_address.sin_addr,
            ip_buffer,
            sizeof(ip_buffer)
        );
        std::string client_ip = ip_result != nullptr ? ip_buffer : "unknown";

        //handle client request with client socket
        //enq request to task queue for worker threads to handle client
        //lambda expression to pass server and client socket fd
        pool.enqueue([this, client_fd, client_ip](){
            handle_client(client_fd, client_ip);
        });
    }
}

