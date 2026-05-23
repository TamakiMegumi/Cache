#pragma once
#include <string>
#include <map>
#include <set>
#include <ctime>
#include "utils.h"

struct Entry
{
    std::string value;
    time_t expire_time; // 0 = never expire
};

class Cache
{
private:
    std::map<std::string, Entry> cache;
    std::map<time_t, std::set<std::string>> expire_index;

public:
    void Set(const std::string &key, const std::string &value, time_t expire);
    std::string Get(const std::string &key);
    bool Del(const std::string &key);
    int64_t Ttl(const std::string &key);
    void RemoveExpiredKeys();
};

extern Cache global_cache;