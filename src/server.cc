#include "server.h"
int createServerSocket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket create failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5))
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port 6379 ..." << std::endl;
    return server_fd;
}
void addEpollEvent(int epfd, int fd, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        perror("epoll_ctl add failed");
    }
}
void removeEpollEvent(int epfd, int fd)
{
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) < 0)
    {
        if (errno != EBADF)
        {
            perror("epoll_ctl del failed");
        }
    }
}
void closeClientConnection(int epfd, int fd, std::vector<int> &clients,
                           std::map<int, std::string> &client_buffers)
{
    removeEpollEvent(epfd, fd);
    close(fd);

    auto it = std::find(clients.begin(), clients.end(), fd);
    if (it != clients.end())
    {
        clients.erase(it);
    }

    client_buffers.erase(fd);
}
void sendResponse(int client_fd, const std::string &msg)
{
    // 确保消息以 \r\n 结尾 (Redis 协议)
    std::string final_msg = msg;
    if (final_msg.size() < 2 || final_msg.substr(final_msg.size() - 2) != "\r\n")
    {
        final_msg += "\r\n";
    }

    ssize_t sent = send(client_fd, final_msg.data(), final_msg.size(), 0);
    if (sent < 0)
    {
        std::cerr << "Send error to fd " << client_fd << std::endl;
    }
    else
    {
        std::cout << "Sent to fd " << client_fd << ": " << msg << std::endl;
    }
}

void handleClientData(int epfd, int fd, std::vector<int> &clients,
                      std::map<int, std::string> &client_buffers, Cache &global_cache)
{
    char buffer[BUFFER_SIZE];
    ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0)
    {
        closeClientConnection(epfd, fd, clients, client_buffers);
    }
    buffer[n] = '\0';
    std::string &buf = client_buffers[fd];
    if (buf.size() + n > MAX_CLIENT_BUFFER_SIZE)
    {
        std::cerr << "Client buffer overflow for fd " << fd << ", closing connection." << std::endl;
        closeClientConnection(epfd, fd, clients, client_buffers);
        return;
    }
    buf.append(buffer, n);
    size_t pos;
    while ((pos = buf.find('\n')) != std::string::npos)
    {
        std::string cmd = buf.substr(0, pos);
        if (!cmd.empty() && cmd.back() == '\r')
        {
            cmd.pop_back();
        }
        buf.erase(0, pos + 1);
        if (!cmd.empty())
        {
            handleCommand(fd, cmd, global_cache);
        }
    }
}