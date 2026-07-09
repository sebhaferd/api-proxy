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
)