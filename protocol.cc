#include "protocol.h"
bool parseSetCommand(const std::string &cmd, std::string &key, std::string &value, int &expireSec)
{
    std::istringstream iss(cmd);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    if (tokens.size() < 3)
    {
        return false;
    }

    // 忽略大小写比较命令头
    std::string cmdHead = tokens[0];
    std::transform(cmdHead.begin(), cmdHead.end(), cmdHead.begin(), ::toupper);
    if (cmdHead != "SET")
    {
        return false;
    }
    key = tokens[1];
    value = tokens[2];
    expireSec = 0;

    if (tokens.size() == 5)
    {
        if (tokens[3] == "EX" || tokens[3] == "ex")
        {
            try
            {
                expireSec = std::stoi(tokens[4]);
                if (expireSec <= 0)
                {
                    return false;
                }
            }
            catch (...)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else if (tokens.size() > 3)
    {
        return false;
    }
    return true;
}