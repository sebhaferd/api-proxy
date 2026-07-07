#pragma once
#include <iostream>
#include <string>

class HttpRequest{
public:
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;

    static HttpRequest parse(const std::string& data);
};


