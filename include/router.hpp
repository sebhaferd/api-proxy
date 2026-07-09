#pragma once
#include <iostream>
#include <string>
#include <vector>

struct RouteTarget{
    std::string host;
    std::string port;
};

struct Route{
    std::string prefix;
    RouteTarget target;
    std::unordered_map<std::string, std::string> injected_headers;
};

class Router{
public:
    Router();
    bool resolve(const std::string& path, Route& matched_route) const;

private:
    std::vector<Route> json_to_routes(const std::string& file_name);
    std::vector<Route> routes = json_to_routes("config/routes.json");
};

