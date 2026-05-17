#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <sys/epoll.h>
#include <algorithm>
#include "cache.h"
#include "protocol.h"
#include "handler.h"
#include "server.h"
Cache global_cache;
std::map<int, std::string> client_buffers;
int main(int argc, char *argv[])
{
    int server_fd = createServerSocket(PORT);

    int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        perror("epoll_create1 failed");
        return 1;
    }

    addEpollEvent(epfd, server_fd, EPOLLIN);

    std::vector<int> clients;
    time_t last_cleanup_time = time(nullptr);
    epoll_event events[MAX_EVENTS];

    while (true)
    {
        // 1. 定时清理过期键
        time_t now = time(nullptr);
        if (now - last_cleanup_time >= CLEANUP_INTERVAL_SEC)
        {
            global_cache.RemoveExpiredKeys();
            last_cleanup_time = now;
        }

        // 2. 等待事件
        // 设置 timeout 为 1000ms，以便即使没有网络事件也能定期执行上面的清理逻辑
        // 如果设为 -1，且没有网络事件，清理逻辑可能会延迟执行
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, 1000);

        if (nfds < 0)
        {
            if (errno == EINTR)
                continue; // 被信号中断，重试
            perror("epoll_wait failed");
            break;
        }

        // 3. 处理事件
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;

            if (fd == server_fd)
            {
                // 新连接
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd >= 0)
                {
                    clients.push_back(client_fd);
                    client_buffers[client_fd] = ""; // 初始化缓冲区
                    addEpollEvent(epfd, client_fd, EPOLLIN);
                }
                else
                {
                    perror("accept failed");
                }
            }
            else
            {
                // 客户端数据
                handleClientData(epfd, fd, clients, client_buffers, global_cache);
            }
        }
    }

    close(server_fd);
    close(epfd);
    return 0;
}
