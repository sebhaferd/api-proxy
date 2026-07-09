# Code Roadmap:

## Standard Progression to API Proxy

### **v0.1:** Began with basic HTTP Server
<details>
<summary>Details</summary>

Goal: Build basic server able to accept TCP connections and parse HTTP requests
Initialize Server socket <br>
Listen for TCP connections <br>
Initialize client socket <br>
Parse HTTP request <br>
Return hardcoded response to client terminal <br>
</details>

### **v0.2:** Implemented forward proxy with hardcoded destination
<details>
<summary>Details</summary>
Goal: Forward between client and destination sockets, Parse requests and responses
Parse client HTTP request <br>
Hardcode target port and destination <br>
Initialize dest socket <br>
Send request to dest <br>
Forward response back to client. <br>
</details>

### **v0.3:** Implemented routing for forward request destinations 
<details>
<summary>Details</summary>
Goal: Forward request to destination API through proxy
Implement router storing dest, port, prefix <br>
Resolve destination host and port by matching prefix against configured routes <br>
Send request to dest and forward back to client <br>
</details>

### **v0.4:** Implemeted logging into log file
<details>
<summary>Details</summary>
Goal: Log requests and relevant information as clients send them to server
Measure end to end proxy latency of each client request <br>
Write request data, response size, latency, and status to logging output file <br>
</details>

### **v0.5:** Configuration based routing
<details>
<summary>Details</summary>
Goal: Have routes stored with given prefix to automate routing to destination
Wrote a `.json file` to store prefix, host/port pairs <br>
Parse JSON configuration into `Router` objects <br>
- Note add CLI feature later to update routes config <br>
</details>

### **v0.6:** API key injection
<details>
<summary>Details</summary>  
Goal: Have proxy handle forwarding API Keys and other important headers<br>
Store route specific API keys and additional header information in route config<br>
Inject headers into client HTTP request before forwarding to dest <br>
</details>

### **v0.7:** thread pool/concurrency
<details>
<summary>Details</summary> 
Goal: Handle clients concurrently <br> 
Implemented fixed size `ThreadPool` to handle shared task queue<br>
Main server thread accepts connection from client, enqueues handle_client() to tasks<br>
Workers wait on std::condition_variable until available tasks <br>
Utilize mutex to lock queue to ensure safe thread access in shared task queue <br>
Wake all workers and join each worker thread in destructor <br>
</details>

### **v0.8:** SQL request logging
<details>
<summary>Details</summary> 
Goal: Replace file based logging with database to store log requests <br> 
Created SQL table storing log data, including request, response size, latency <br>
Built `SqlLogger` class to connect to Postgres using `libpq` and insert log entries <br>
Containerized Postgres using docker for a portable environment <br>
Pass each log request parameter into row as value using placeholder <br>
Protect shared database writes with `std::mutex` with concurrent worker threads <br>
</details>

- ### **v0.9:** admin logs
<details>
<summary>Details</summary> 
Implemented admin logs endpoint to return last 10 request logs from database<br>
Recieve HTTP request from admin -> parse sql table to json output response <br>
</details>

- **v1.0:** caching
- **v1.1:** rate limiting
- **v1.2:** HTTPS


## Additional version extensions

