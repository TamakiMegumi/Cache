#pragma once
#include "cache.h"
#include "protocol.h"
void handleCommand(int client_fd, const std::string &cmd, Cache &cache);