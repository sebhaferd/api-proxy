#pragma once
#include <string>
#include <iostream>
#include <fstream>


class Logger{
public:
    explicit Logger(const std::string& file_path);
    void log_request(
        const std::string& method,
        const std::string& origin_path,
        const std::string& target_host,
        const std::string& forward_path,
        int status_code,
        bool headers_injected,
        long latency_ms,
        size_t response_size
    );
private:
    std::string file_path;
};