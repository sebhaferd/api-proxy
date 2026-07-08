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
    std::vector<Route> json_to_routes(const std::string& file_name);
    std::vector<Route> routes = json_to_routes("config/routes.json");
};

