# Code Roadmap:

## Standard Progression to API Proxy

- **v0.1:** Began with basic HTTP Server
    <details>
    <summary>Details</summary>
    Initialize Server socket
    Listen for TCP connections
    Initialize client socket
    Parse HTTP request
    Return hardcoded response to client terminal
    </details>

- **v0.2:** Implemented forward proxy with hardcoded destination
    <details>
    <summary>Details</summary>
    Parse client HTTP request
    Hardcode target port and destination
    Initialize dest socket
    Send request to dest
    Forward response back to client.
    </details>

- **v0.3:** Implemented routing for forward request destinations 
    <details>
    <summary>Details</summary>
    Implement router storing dest, port, prefix
    Search router vector using prefix to find destination
    Send request to dest and forward back to client. 
    </details>
- **v0.4:** Implemeted logging into log file
    <details>
    <summary>Details</summary>
    Measure latency of client interaction
    Stream log request, response, latency, and status to logging output file
    </details>

- **v0.5:** API key injection
- **v0.6:** rate limiting
- **v0.7:** caching
- **v0.8:** concurrency/thread pool
- **v0.9:** SQL logging

## Additional version extensions

