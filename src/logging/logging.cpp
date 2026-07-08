#include "../../include/logging.hpp"

Logger::Logger(const std::string& file_path)
    : file_path(file_path) {}

void Logger::log_request(
        const std::string& method,
        const std::string& origin_path,
        const std::string& target_host,
        const std::string& forward_path,
        int status_code,
        long latency_ms,
        size_t response_size
    ){
        std::ofstream log(file_path, std::ios::app);

        if (!log){
            std::cerr<< "log file failed to open";
            return;
        }
        log << method << " " << 
            origin_path << " -> "<<
            target_host << " " <<
            forward_path << " " <<
            "status = " << status_code << 
            " latency = " << latency_ms << " ms " <<
            "size = " << response_size<<std::endl;
}