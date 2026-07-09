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

#include "../../include/server.hpp"
#include "../../include/http-request.hpp"
#include "../../include/router.hpp"
#include "../../include/thread-pool.hpp"


Server::Server(int port)
    : port(port), server_fd(-1), logger() {}

void Server::start(){
    if (!setup_socket()){
        std::cerr<<"Server setup failed"<<std::endl;
        return;
    }
    accept_loop();
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

//forward http request to server
std::string forward_to_server(const RouteTarget& target, 
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
    std::ostringstream request;
    request << req.method << " " << forward_path << " " << req.version << "\r\n";

    for (const auto& [key, val] : req.headers) {
        request << key << ": " << val << "\r\n";
    }
    std::string request_str = request.str();
    send(forward_fd, request_str.c_str(), request_str.size(), 0);

    //recieve response from example.com
    std::string response;
    char buffer[4096];
    ssize_t bytes_recieved;
    while ((bytes_recieved = recv(forward_fd, buffer, sizeof(buffer), 0)) > 0){
        response.append(buffer, bytes_recieved);
    }

    //free addrinfo ptr and close socket
    freeaddrinfo(result);
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

void Server::return_recent_logs(int client_fd){
    std::string body = logger.get_recent_logs_json();
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-length: " + std::to_string(body.size()) + "\r\n"
        + body;
    send(client_fd, response.c_str(), response.size(), 0);
    close(client_fd);
    return;
}

void Server::error404(int client_fd){
    std::string body = "Route not found";
    std::string not_found = 
        "HTTP/1.1 404 not found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "\r\n"
        + body;
    
    send(client_fd, not_found.c_str(), not_found.size(), 0);
    close(client_fd);
}
void Server::handle_client(int client_fd){
    //concurrency testing: output client socket ind and current thread
    std::cout << "client_fd=" << client_fd
          << " thread=" << std::this_thread::get_id()
          << std::endl;

    //set char buffer for input
    char buffer[4096] = {0};
    //start clock to measure latency
    auto start = std::chrono::system_clock::now();
    //read bytes from client socket
    ssize_t bytes_read = recv(client_fd, (void*)buffer, sizeof(buffer)-1, 0);
    if (bytes_read < 0){
        std::cerr<< "recv failed";
        return;
    }
    if (bytes_read == 0){
        std::cerr<< "Client closed connection";
        return;
    }

    //convert bytes to string
    std::string raw_data(buffer, bytes_read);
    HttpRequest req = HttpRequest::parse(raw_data);
    if (req.path == "/admin/logs"){
        return_recent_logs(client_fd);
        return;
    }
    //Get destination for result from router vector
    Route route;
    if (!router.resolve(req.path, route)){
        error404(client_fd);
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
    

    //send response to client
    std::string response = forward_to_server(route.target, forward_path, req);

    //parse for status code from application response, eg. 200 OK
    int status_code = get_status_code(response);
    //measure latency from client request intialization to response 
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    long latency = elapsed.count();

    //log request
    logger.log_request(req.method, req.path, route.target.host, 
                        forward_path, status_code, headers_injected, 
                        latency, response.size());

    send(client_fd, response.c_str(), response.size(), 0);
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

        //handle client request with client socket
        //enq request to task queue for worker threads to handle client
        //lambda expression to pass server and client socket fd
        pool.enqueue([this, client_fd](){
            handle_client(client_fd);
        });
    }
}

