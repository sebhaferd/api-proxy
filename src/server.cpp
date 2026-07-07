#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include "../include/server.hpp"
#include "../include/http-request.hpp"


Server::Server(int port)
    : port(port), server_fd(-1){}

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
std::string forward_to_server(const HttpRequest& req, const std::string& raw_data){
    //temp hardcoded destination
    std::string dest_host = "example.com";
    std::string dest_port = "80";
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
    

    //send request to example.com
    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(forward_fd, request.c_str(), request.size(), 0);

    //recieve response from example.com
    std::string response;
    char buffer[4096];
    ssize_t bytes_recieved;
    while (bytes_recieved = recv(forward_fd, buffer, sizeof(buffer), 0) > 0){
        response.append(buffer, bytes_recieved);
    }

    //free addrinfo ptr and close socket
    freeaddrinfo(result);
    close(forward_fd);
    return response;
    
}

void Server::handle_client(int client_fd){
    //set char buffer for input
    char buffer[4096] = {0};
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

    //output request
    std::cout<<"Method:" << req.method<<"\n";
    std::cout<<"Path:"<<req.path<<"\n";
    std::cout<<"Version:"<<req.version<<std::endl;
    std::cout<<"Headers:"<<std::endl;
    for (const auto [key, val] : req.headers){
        std::cout<<key<<": "<<val<<std::endl;
    }

    //send response to client
    std::string response = forward_to_server(req, raw_data);
    send(client_fd, response.c_str(), response.size(), 0);
    
}


void Server::accept_loop(){
    //wait for client to connect to port
    while (true){
        sockaddr_in client_address{};
        socklen_t address_size = sizeof(client_address);
        int client_fd = accept(server_fd, (sockaddr*)&client_address, &address_size);
        if (client_fd < 0){
            std::cerr<<"Accept failed\n";
            continue;
        }
        //handle client request with client socket
        handle_client(client_fd);
        close(client_fd);
    }
}

