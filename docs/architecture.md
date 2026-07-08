# Code architecture:

```text
Client -> Proxy -> destination API 
destination API -> Proxy -> Client

##Features of Proxy:
- Parse Http Request
- Route request to destination API
- Forward request to destination API
- Send back response to client
- log request details and latency