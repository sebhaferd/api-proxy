# Code Roadmap:

## Standard Progression to API Proxy

- **v0.1:** Began with basic HTTP Server
    <details>
    <summary>Details</summary>
    Initialize Server socket <br>
    Listen for TCP connections <br>
    Initialize client socket <br>
    Parse HTTP request <br>
    Return hardcoded response to client terminal <br>
    </details>

- **v0.2:** Implemented forward proxy with hardcoded destination
    <details>
    <summary>Details</summary>
    Parse client HTTP request <br>
    Hardcode target port and destination <br>
    Initialize dest socket <br>
    Send request to dest <br>
    Forward response back to client. <br>
    </details>

- **v0.3:** Implemented routing for forward request destinations 
    <details>
    <summary>Details</summary>
    Implement router storing dest, port, prefix <br>
    Search router vector using prefix to find destination <br>
    Send request to dest and forward back to client <br>
    </details>
- **v0.4:** Implemeted logging into log file
    <details>
    <summary>Details</summary>
    Measure latency of client interaction <br>
    Stream log request, response, latency, and status to logging output file <br>
    </details>

- **v0.5:** Configuration based routing
    <details>
    <summary>Details</summary>
    Measure latency of client interaction <br>
    Stream log request, response, latency, and status to logging output file <br>
    -Note add CLI feature later
    </details>
- **v0.6:** API key injection
- **v0.7:** rate limiting
- **v0.8:** caching
- **v0.9:** concurrency/thread pool
- **v1.0:** SQL logging

## Additional version extensions

