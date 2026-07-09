#include "../../include/sql-logger.hpp"
#include <iostream>
#include <sstream>

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
        //check status of database before logging
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Database not connected\n";
            return;
        }
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
            return;
        }
        if (res == nullptr) {
            std::cerr << "Insert failed: " << PQerrorMessage(conn);
            return;
        }
        std::cout<<"Request log successfully inserted: ";
        PQclear(res);
    }

std::string pg_res_to_json(PGresult *res){
    int numrows = PQntuples(res);
    int numcols = PQnfields(res);

    //create output json string to write to
    std::ostringstream json_out;
    json_out << "[";
    //parse table into json string
    for (int i = 0; i < numrows; i++) {
        if (i > 0) {
            json_out << ",";
        }

        json_out << "{";

        for (int j = 0; j < numcols; j++) {
            if (j > 0) {
                json_out << ",";
            }

            std::string col_name = PQfname(res, j);

            json_out << "\"" << col_name << "\":";

            if (PQgetisnull(res, i, j)) {
                json_out << "null";
            } else {
                std::string col_val = PQgetvalue(res, i, j);
                json_out << "\"" << col_val << "\"";
            }
        }

        json_out << "}";
    }
    
    json_out << "]";
    return json_out.str();

}

std::string SqlLogger::get_recent_logs_json(){
    //lock db using mutex to read
    std::lock_guard<std::mutex> lock(db_mutex);
    //read 10 most recent logs from db
    std::string query = "SELECT * FROM request_logs ORDER BY id DESC LIMIT 10;";
    //pass query to postgres
    PGresult* res = PQexec(conn, query.c_str());
    //check if read worked
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Log read failed: " << PQerrorMessage(conn);
        PQclear(res);
        return "[]";
    }
    std::string json_out = pg_res_to_json(res);
    PQclear(res);
    return json_out; 
}
