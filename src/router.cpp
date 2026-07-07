#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include "../include/router.hpp"


Router::Router() = default;

bool Router::resolve(const std::string& path, RouteTarget& target) const{
    for (const Route& route : routes){
        if (path.rfind(route.prefix, 0) == 0){
            target = route.target;
            return true;
        }
    }
    return false;
}