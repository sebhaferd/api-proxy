# Code architecture:


#### Client -> Proxy -> Destination API 
#### Destination API -> Proxy -> Client

## Features of Proxy:
- Parse Http Request
- Route request to destination API
- Forward request to destination API
- Send back response to client
- log request details and latency
- concurrently handle client requests
- store log requests in SQL database
- lru eviction caching using in memory cache
