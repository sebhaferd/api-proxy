#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <string_view>

#include "../../include/json.hpp"
#include "../../include/router.hpp"

using json = nlohmann::json;

Router::Router() = default;

std::vector<Route> Router::json_to_routes(const std::string& file_name){
    std::ifstream file(file_name);
    if (!file.is_open()){
        std::cerr<<"Unable to open router configuration";
        return {};
    }
    std::vector<Route> result;
    try{
        json routes = json::parse(file);
        for (auto& [key, val] : routes.items()){
            Route route;
            route.prefix = key;
            route.target.host = val.at("host").get<std::string>();
            route.target.port = val.at("port").get<std::string>();
            result.push_back(route);
        }
        
    }
    catch (const json::exception& e) {
        // Handle syntax or structural errors in the JSON file
        std::cerr << "JSON Routes Error: " << e.what() << std::endl;
        return {};
    }
    return result;
}

bool Router::resolve(const std::string& path, RouteTarget& target) const{
    for (const Route& route : routes){
        if (path.rfind(route.prefix, 0) == 0){
            target = route.target;
            target.prefix = route.prefix;
            return true;
        }
    }
    return false;
}