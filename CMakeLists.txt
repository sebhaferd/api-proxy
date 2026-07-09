cmake_minimum_required(VERSION 3.10)
project(api-proxy)

set(CMAKE_CXX_STANDARD 20)

add_executable(proxy 
    src/proxy/server.cpp
    src/main.cpp
    src/proxy/http-request.cpp
    src/proxy/router.cpp
    src/logging/logger.cpp
    src/proxy/thread-pool.cpp
    src/logging/sql-logger.cpp
)

target_include_directories(proxy PRIVATE /opt/homebrew/opt/libpq/include)
target_link_directories(proxy PRIVATE /opt/homebrew/opt/libpq/lib)
target_link_libraries(proxy pq)