#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>
#include <functional>
#include "cache_base.h"
namespace CacheSpace
{
    template <typename Key, typename Value>
    class LRUcache;
    template <typename Key, typename Value>
    class LRUNode
    {

    private:
        Key key;
        Value val;
        size_t accessCount;
        std::weak_ptr<LRUNode<Key, Value>> prev;
        std::shared_ptr<LRUNode<Key, Value>> next;

    public:
        LRUNode(Key key, Value val) : key(key), val(val), accessCount(0), prev(nullptr), next(nullptr) {}

        Key getKey() { return key; }
        Value getVal() { return val; }
        size_t getAccessCount() { return accessCount; }
        void setValue(const Value &val) { this->val = val; }
        void incrementAccessCount() { accessCount++; }
        friend class LRUcache<Key, Value>;
    };

    template <typename Key, typename Value>
    class LRUcache : public CacheBase<Key, Value>
    {
    protected:
        int capacity;
        node_map nodeMap;
        std::mutex mut;
        node_ptr dummyHead;
        node_ptr dummyTail;

    public:
        using node_t = LRUNode<Key, Value>;
        using node_ptr = std::shared_ptr<node_t>;
        using node_map = std::unordered_map<Key, node_ptr>;
        LRUcache(size_t capacity) : capacity(capacity)
        {
            init();
        }
        ~LRUcache() override = default;

        void put(Key key, Value val) override
        {
            if (capacity <= 0)
            {
                return;
            }
            std::lock_guard<std::mutex> lock(mut);
            auto it = nodeMap.find(key);
            if (it != nodeMap.end())
            {
                updateExistingNode(it->second, val);
                return;
            }
            addNewNode(key, val);
        }
        bool get(Key key, Value &value) override
        {
            std::lock_guard<std::mutex> lock(mut);
            auto it = nodeMap.find(key);
            if (it != nodeMap.end())
            {
                moveToMostRecent(it->second);
                value = it->second->getVal();
                return true;
            }
            return false;
        }

        Value get(Key key) override
        {
            Value value{};
            get(key, value);
            return value;
        }

        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock(mut);
            auto it = nodeMap.find(key);
            if (it != nodeMap.end())
            {
                removeNode(it->second);
                nodeMap.erase(it);
            }
        }

        void init()
        {
            dummyHead = std::make_shared<node_t>(Key{}, Value{});
            dummyTail = std::make_shared<node_t>(Key{}, Value{});
            dummyHead->next = dummyTail;
            dummyTail->prev = dummyHead;
        }

        void updateExistingNode(node_ptr node, const Value &val)
        {
            node->setValue(val);
            moveToMostRecent(node);
        }

        void addNewNode(const Key &key, const Value &val)
        {
            if (nodeMap.size() >= capacity)
            {
                evictLeastRecent();
            }
            node_ptr newNode = std::make_shared<node_t>(key, val);
            insertNode(newNode);
            nodeMap[key] = newNode;
        }

        void moveToMostRecent(node_ptr node)
        {
            removeNode(node);
            insertNode(node);
        }

        void removeNode(node_ptr node)
        {
            if (!node->prev.expired() && node->next)
            {
                auto prev = node->prev.lock();
                prev->next = node->next;
                node->next->prev = prev;
                node->next = nullptr;
            }
        }

        void insertNode(node_ptr node)
        {
            node->next = dummyTail;
            node->prev = dummyTail->prev;
            dummyTail->prev.lock()->next = node;
            dummyTail->prev = node;
        }

        void evictLeastRecent()
        {
            node_ptr leastRecent = dummyHead->next;
            removeNode(leastRecent);
            nodeMap.erase(leastRecent->getKey());
        }
    };

    template <typename Key, typename Value>
    class LRUKcache : public LRUcache<Key, Value>
    {
    protected:
        int k;
        std::unique_ptr<LRUcache<Key, size_t>> hisList;
        std::unordered_map<Key, Value> hisValMap;

    public:
        LRUKcache(int capacity, int hisCapacity, in t k)
            : LRUcache<Key, Value>(capacity),
              hisList(std::make_unique<LRUcache<Key, size_t>>(hisCapacity)), k(k) {};
        ~LRUKcache() override = default;
        Value get(Key key) override
        {
            Value value{};
            bool inMainCache = LRUcache<Key, Value>::get(key, value);
            size_t hisCnt = hisList->get(key);
            if (inMainCache)
            {
                return value;
            }
            if (hisCnt >= k)
            {
                auto it = hisValMap.find(key);
                if (it != hisValMap.end())
                {
                    Value storedVal = it->second;
                    hisList->remove(key);
                    hisValMap.erase(it);
                    LRUcache<Key, Value>::put(key, storedVal);
                    return storedVal;
                }
            }
            return value;
        }

        void put(Key key, Value val) override
        {
            Value existVal{};
            bool inMainCache = LRUcache<Key, Value>::get(key, existVal);
            if (inMainCache)
            {
                LRUcache<Key, Value>::put(key, val);
                return;
            }
            size_t hisCnt = hisList->get(key);
            hisCnt++;
            hisList->put(key, hisCnt);
            hisValMap[key] = val;
            if (hisCnt >= k)
            {
                hisList->remove(key);
                hisValMap.erase(key);
                LRUcache<Key, Value>::put(key, val);
            }
        }
    };

    template <typename Key, typename Value>
    class HashLRUcache : public CacheBase<Key, Value>
    {
    protected:
        size_t capacity;
        int slice;
        std::vector<std::unique_ptr<LRUcache<Key, Value>>>
            lruSliceCaches;
        size_t Hash(Key key)
        {
            return std::hash<Key>()(key);
        }

    public:
        HashLRUcache(size_t capacity, int slice)
            : capacity(capacity),
              slice(slice > 0 ? slice : std::thread::hardware_concurrency())
        {
            size_t sliceSize = std::ceil(capacity / static_cast<double>(slice));
            lruSliceCaches.resize(slice, std::make_unique<LRUcache<Key, Value>>(sliceSize));
        }
        ~HashLRUcache() override = default;
        bool get(Key key, Value &value) override
        {
            size_t sliceIndex = Hash(key) % slice;
            return lruSliceCaches[sliceIndex]->get(key, value);
        }
        Value get(Key key) override
        {
            Value value{};
            get(key, value);
            return value;
        }
    };
}
