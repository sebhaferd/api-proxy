#include <iostream>
#include <sstream>
#include "../../include/http-request.hpp"

//Parse client http request and return mathod path version and additional headers
HttpRequest HttpRequest::parse(const std::string& data){
    HttpRequest req;


    //get lines
    std::istringstream stream(data);
    std::string request_line;
    std::getline(stream, request_line);

    //parse line request
    std::istringstream line_stream(request_line);

    line_stream >> req.method;
    line_stream >> req.path;
    line_stream >> req.version;

    //read headers
    std::unordered_map<std::string, std::string> headers;
    std::string header_line;
    while (std::getline(stream, header_line)){
        if (header_line == "\r" || header_line.empty()) break;
        size_t colon_ind = header_line.find(":");
        if (colon_ind == std::string::npos) continue;

        std::string key, val;
        key = header_line.substr(0, colon_ind);
        val = header_line.substr(colon_ind+1);
        if (!val.empty() && val[0] == ' '){
            val.erase(0, 1);
        }

        if (!val.empty() && val.back() == '\r'){
            val.pop_back();
        }
        req.headers[key] = val;
    }
    return req;
}

