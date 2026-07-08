# Design Descisions:

## Version 1: Basic Implementation

#### HTTP headers: `std::unordered_map` -> quick lookup of header name val pairs
#### ROUTER routes: `std::vector`-> simple implementation for routes to be checked in order
#### ROUTER config: `.json file` -> store prefix: host, port in json file for importing large number of destinations

#### LOGGER log requests: `.log file` -> requests written to log file for simple logging of request response and latency

