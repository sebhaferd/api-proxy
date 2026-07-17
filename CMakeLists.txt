cmake_minimum_required(VERSION 3.16)

project(reverse_proxy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_executable(proxy
    src/proxy/server.cpp
    src/main.cpp
    src/proxy/http-request.cpp
    src/proxy/router.cpp
    src/proxy/thread-pool.cpp
    src/logging/logger.cpp
    src/logging/sql-logger.cpp
    src/caching/cache.cpp
    src/rate-limiting/rate-limiter.cpp
)

target_include_directories(
    proxy
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    /opt/homebrew/opt/libpq/include
)

target_link_directories(
    proxy
    PRIVATE
    /opt/homebrew/opt/libpq/lib
)

target_link_libraries(
    proxy
    PRIVATE
    pq
    OpenSSL::SSL
    OpenSSL::Crypto
)