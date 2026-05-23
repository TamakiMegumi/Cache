#pragma once
#include <iostream>
#include <cinttypes>
#include <map>
const int PORT = 6379;
const int MAX_EVENTS = 1024;
const int BUFFER_SIZE = 1024;
const int CLEANUP_INTERVAL_SEC = 5;
const size_t MAX_CLIENT_BUFFER_SIZE = 1024 * 1024;
extern std::map<int, std::string> client_buffers;