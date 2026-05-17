# KamaCache - A Lightweight In-Memory Cache Server

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