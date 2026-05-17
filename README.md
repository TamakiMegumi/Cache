# Cache - A Lightweight In-Memory Cache Server

A high-performance, single-threaded key-value storage system implemented in **C++**, inspired by Redis. This project demonstrates the core principles of Linux network programming, including IO multiplexing (Epoll), custom protocol parsing, and efficient memory management.

## 🚀 Features

- **High Concurrency**: Built on Linux `epoll` IO multiplexing model to handle thousands of concurrent connections efficiently.
- **Redis-like Protocol**: Supports a subset of the RESP (Redis Serialization Protocol) for client-server communication.
- **Core Commands**:
  - `SET`, `GET`, `DEL`: Basic CRUD operations.
  - `EXPIRE`, `TTL`: Time-to-Live management for automatic key expiration.
  - `PING`: Connectivity check.
- **Memory Management**: Efficient handling of client buffers to prevent memory leaks and handle TCP packet sticking/splitting.
- **Modular Design**: Clean separation of concerns into Network Layer server, Business Logic (`handler`), and Data Structure cache.

## 🛠️ Tech Stack

- **Language**: C++17
- **OS**: Linux (Ubuntu 22.04)
- **Network**: POSIX Sockets, Epoll
- **Data Structures**: `std::map`, `std::set` for O(log n) lookups and ordered expiration tracking.

## 📦 How to Build & Run

### Prerequisites
- G++ compiler supporting C++17
- Linux environment

### Compilation
```bash
g++ main.cc cache.cc protocol.cc handler.cc server.cc -o cache -std=c++17
```

### Running the Server
```bash
./cache
```
### Testing with Telnet
```bash
telnet 127.0.0.1 6379
```
### Example Usage
```bash
SET name young
+OK
GET name
$6
young
EXPIRE name 10
:1
TTL name
:9
```
### 🏗️Project Structure
File|Description
|----|-----------|
`cache.h/cc`|	Core data structure implementing the Key-Value store and expiration logic.
`server.h/cc`	|Network layer handling Socket creation, Epoll event loop, and connection management.
`handler.h/cc`	|Business logic layer parsing commands and interacting with the Cache.
`protocol.h/cc`	|Utility functions for parsing specific command formats (e.g., SET with EX).
|||

### 💡 Learning Outcomes
This project was developed as part of the KamaCache learning path. Key takeaways include:

- Understanding the difference between Blocking IO and IO Multiplexing.
- Handling TCP stickiness and packet splitting in application-layer protocols.
- Designing a clean, modular architecture in C++.

### 📝 License
This project is for educational purposes.

