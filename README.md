# API Gateway Proxy

This project receives API HTTP requests from client applications, forwards them to the appropriate service, and returns the response

## Current Features

- HTTP request forwarding
- Request routing
- JSON file route configuration
- API key and additional header injections
- Logging client requests
- Logging performance and latency metrics
- Thread pool for handling clients concurrently

## Planned Features

- Rate limiting
- Response caching
- HTTPS/TLS
- SQL logging
- Simple dashboard
- Traffic monitoring

## Purpose

This project is being built as a learning project to explore:

- Networking
- Systems Programming
- Distributed systems
- Backend Infrastructure
- Concurrent Programming
- Security Engineering

## How to run project:

Initialize proxy server:
- $cmake --build build
- $./build/proxy

Send proxy request:
- $curl http:/example.com

