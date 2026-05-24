#include "cache.h"
#include <iostream>
/**set key and value
 * if expire time is 0
 * expire_index not insert key
 * expire is current timestamp
 */
void Cache::Set(const std::string &key, const std::string &value,
                time_t expire)
{
    auto it = cache.find(key);
    if (it != cache.end() && it->second.expire_time != 0)
    {
        auto &keys = expire_index[it->second.expire_time];
        keys.erase(key);
        if (keys.empty())
        {
            expire_index.erase(it->second.expire_time);
        }
    }
    cache[key] = {value, expire};
    if (expire != 0)
    {
        expire_index[expire].insert(key);
    }
}

/**get value from key
 * if not exist key or expire time
 * return empty string: ""
 */
std::string Cache::Get(const std::string &key)
{
    auto it = cache.find(key);
    if (it == cache.end())
    {
        return "";
    }
    time_t now = time(nullptr);
    if (it->second.expire_time != 0 && now >= it->second.expire_time)
    {
        expire_index[it->second.expire_time].erase(key);
        cache.erase(it);
        return "";
    }
    return it->second.value;
}

/*Delete the key*/
bool Cache::Del(const std::string &key)
{
    auto it = cache.find(key);
    if (it == cache.end())
    {
        return false;
    }
    if (it->second.expire_time != 0)
    {
        expire_index[it->second.expire_time].erase(key);
    }
    cache.erase(it);
    return true;
}

/**return the ttl of the key
 * if the key is not existed or expires return -2
 * if the key never expires return -1
 */
int64_t Cache::Ttl(const std::string &key)
{
    auto it = cache.find(key);
    if (it == cache.end())
    {
        return -2;
    }
    if (it->second.expire_time == 0)
    {
        return -1;
    }
    time_t now = time(nullptr);
    if (now >= it->second.expire_time)
    {
        Del(key);
        return -2;
    }
    return static_cast<int64_t>(it->second.expire_time - now);
}
void Cache::RemoveExpiredKeys()
{
    time_t now = time(nullptr);
    auto it = expire_index.begin();
    while (it != expire_index.end() && it->first <= now)
    {
        for (const auto &key : it->second)
        {
            cache.erase(key);
        }
        it = expire_index.erase(it);
    }
}