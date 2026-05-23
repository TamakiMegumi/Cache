#pragma once
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "handler.h"
#include <netinet/in.h>
#include <iostream>
#include <map>
#include <algorithm>
#include "cache.h"
#include "utils.h"

int createServerSocket(int port);
void addEpollEvent(int epfd, int fd, uint32_t events);
void removeEpollEvent(int epfd, int fd);
void closeClientConnection(int epfd, int fd, std::vector<int> &clients,
                           std::map<int, std::string> &client_buffers);
void sendResponse(int client_fd, const std::string &msg);
void handleClientData(int epfd, int fd, std::vector<int> &clients,
                      std::map<int, std::string> &client_buffers, Cache &cache);