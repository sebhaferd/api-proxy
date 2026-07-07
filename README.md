Local API Gateway

A local API gateway that sits between applications and external APIs.

The gateway receives API requests from local applications, forwards them to the appropriate external service, and returns the response while providing additional functionality such as:

Secure API key management
Request and response logging
Performance and latency metrics
Request routing
Rate limiting
Response caching
Configurable security policies
Traffic monitoring

The project is being built in modern C++ as a way to explore networking, backend infrastructure, systems programming, and security engineering from first principles.

How to run project:

Initialize proxy server:
cmake --build build
./build/proxy

Send proxy request:
curl http:/example.com

