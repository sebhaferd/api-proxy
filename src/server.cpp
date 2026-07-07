#include <iostream>
#include <sys/socket.h>
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
    
    if (listen(server_fd, 10) < 0){
        std::cerr<<"Listen failed\n";
        return false;
    }
    std::cout<<"Listening on port"<< port <<std::endl;
    return true;
}

void Server::handle_client(int client_fd){
    //set char buffer for input
    char buffer[4096] = {0};
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

    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World";
    
    send(client_fd, response.c_str(), response.size(), 0);

}


void Server::accept_loop(){
    while (true){
        sockaddr_in client_address{};
        socklen_t address_size = sizeof(client_address);
        int client_fd = accept(server_fd, (sockaddr*)&client_address, &address_size);
        if (client_fd < 0){
            std::cerr<<"Accept failed\n";
            continue;
        }

        handle_client(client_fd);
        close(client_fd);
    }
}