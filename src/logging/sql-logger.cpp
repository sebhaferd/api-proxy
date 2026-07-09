#include "../../include/sql-logger.hpp"
#include <iostream>

SqlLogger::SqlLogger(){
    conn = PQconnectdb(
    "host=localhost port=5432 dbname=proxy_logs user=proxy_user password=proxy_password"
    );
    if (PQstatus(conn) != CONNECTION_OK){
        std::cerr<<"Database connection failed "<<
        PQerrorMessage(conn);
    }
}

SqlLogger::~SqlLogger(){
    if (conn != nullptr){
        PQfinish(conn);
    }
}

void SqlLogger::log_request(
        const std::string& method,
        const std::string& origin_path,
        const std::string& target_host,
        const std::string& forward_path,
        int status_code,
        bool headers_injected,
        long latency_ms,
        size_t response_size
    ){
        //initialize string for postgres query to log requests
        //$1.. used as placeholders for values
        const char* query =
        "INSERT INTO request_logs "
        "(method, origin_path, target_host, forward_path, status_code, latency_ms, response_sz, headers_injected) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8);";
        
        //convert non strings into strings for query
        std::string status = std::to_string(status_code);
        std::string latency = std::to_string(latency_ms);
        std::string size = std::to_string(response_size);
        std::string headers = (headers_injected ? "true" : "false");

        //create string array for input values to query
        const char* values[8] = {
            method.c_str(),
            origin_path.c_str(),
            target_host.c_str(),
            forward_path.c_str(),
            status.c_str(),
            latency.c_str(),
            size.c_str(),
            headers.c_str()
        };
        //lock mutex for shared db to ensure safety with thread access
        std::lock_guard<std::mutex> lock(db_mutex);
        //insert requestlog and values into db
        PGresult* res = PQexecParams(conn, query, 8, nullptr, values, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Insert failed: " << PQerrorMessage(conn);
        }
        else{
            std::cout<<"Request log successfully inserted: ";
        }
        PQclear(res);
}