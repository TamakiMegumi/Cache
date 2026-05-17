#include "handler.h"
#include "server.h"
#include "protocol.h"
#include <iostream>
void handleCommand(int client_fd, const std::string &cmd, Cache &global_cache)
{
    std::string msg;
    time_t current_time = time(nullptr);
    std::istringstream iss(cmd);
    std::string command;
    iss >> command;
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
    if (command == "SET")
    {
        std::string key, value;
        int expireSec = 0;

        if (parseSetCommand(cmd, key, value, expireSec))
        {
            time_t expire_time = 0;
            if (expireSec > 0)
            {
                expire_time = current_time + expireSec;
            }
            global_cache.Set(key, value, expire_time);
            msg = "+OK";
        }
        else
        {
            msg = "-ERR wrong number of arguments for 'set' command";
        }
    }
    else if (command == "GET")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            msg = "-ERR wrong number of arguments for 'get' command";
        }
        else
        {
            std::string val = global_cache.Get(key);
            if (val.empty())
            {
                msg = "$-1";
            }
            else
            {
                msg = "$" + std::to_string(val.length()) + "\r\n" + val;
            }
        }
    }
    else if (command == "DEL")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            msg = "-ERR wrong number of arguments for 'del' command";
        }
        else
        {
            if (global_cache.Del(key))
            {
                msg = ":1";
            }
            else
            {
                msg = ":0";
            }
        }
    }
    else if (command == "TTL")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            msg = "-ERR wrong number of arguments for 'ttl' command";
        }
        else
        {
            int64_t t = global_cache.Ttl(key);
            msg = ":" + std::to_string(t);
        }
    }
    else if (command == "PING")
    {
        msg = "+PONG";
    }
    else
    {
        msg = "-ERR unknown command '" + command + "'";
    }
    msg += "\r\n";
    sendResponse(client_fd, msg);
    // std::cout << "Send to fd" << client_fd << ":" << msg;
}