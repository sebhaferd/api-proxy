# Design Decisions:

## Version 1: Basic Implementation

#### HTTP headers: `std::unordered_map` -> quick lookup of header name val pairs

#### ROUTER routes: `std::vector`-> simple implementation for routes to be checked in order
#### ROUTER config: `.json file` -> store prefix: host, port in json file for importing large number of destinations

#### LOGGER log requests: `.log file` -> requests written to log file for simple logging of request response and latency
#### LOGGER log requests(v2): `sql db` -> insert requests into sql database to handle increasing amount of clients/requests

#### THREADPOOL tasks: `queue` -> FIFO task structure to handle allocate client requests to worker threads
#### THREADPOOL queue: `std::mutex` -> lock queue to ensure safe access across multiple threads sharing task queue

#### RESPONSECACHE cache: `std::unordered_map<string, entry>` -> quick look up for cache hits

