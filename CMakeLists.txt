cmake_minimum_required(VERSION 3.10)
project(api-proxy)

set(CMAKE_CXX_STANDARD 20)

add_executable(proxy 
    src/server.cpp
    src/main.cpp
    src/http-request.cpp
    src/router.cpp
    src/logging/logging.cpp
    )