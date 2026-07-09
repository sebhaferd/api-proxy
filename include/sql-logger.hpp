#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <cstddef>
#include <libpq-fe.h>


class SqlLogger{
public:
    SqlLogger();
    ~SqlLogger();
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
    std::string get_recent_logs_json();

private:
    //initialize connection with postgres db
    PGconn* conn;
    std::mutex db_mutex;
};