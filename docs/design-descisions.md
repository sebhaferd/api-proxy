# Design Descisions:

## Version 1: Basic Implementation

### HTTP headers: `std::unordered_map` -> quick lookup of header name val pairs
### ROUTER routes: `std::vector`-> simple implementation for routes to be checked in order
### LOGGER log requests: `.log file` -> requests written to log file for simple logging of request response and latency