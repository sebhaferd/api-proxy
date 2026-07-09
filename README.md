# API Gateway Proxy

A local API gateway that sits between applications and external APIs.

This gateway receives API HTTP requests from client applications, forwards them to the appropriate service, and returns the response

Intended Functionality includes: 

Secure API key management <br>
Request and response logging <br>
Performance and latency metrics <br>
Request routing <br>
Concurrency to handle clients <br>
Rate limiting <br>
Response caching <br>
TLS protocol <br>


This project is being built as a way to explore networking, backend infrastructure, systems programming, and security engineering as a learning project.

## How to run project:

Initialize proxy server:
- $cmake --build build
- $./build/proxy

Send proxy request:
- $curl http:/example.com

