#pragma once
#include <iostream>
#include <string>
#include <vector>

struct RouteTarget{
    std::string host;
    std::string port;
    std::string prefix;
};

struct Route{
    std::string prefix;
    RouteTarget target;
};

class Router{
public:
    Router();
    bool resolve(const std::string& path, RouteTarget& target) const;

private:
    std::vector<Route> routes = 
    {
        {"/example", {"example.com", "80"}},
        {"/httpbin", {"httpbin.org", "80"}}
    };
};

