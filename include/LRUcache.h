#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "cache_base.h"
namespace Cache
{
    template <typename key_t, typename val_t>
    class LRUcache;
    template <typename key_t, typename val_t>
    class LRUNode
    {

    private:
        key_t key;
        val_t val;
        size_t accessCount;
        std::weak_ptr<LRUNode<key_t, val_t>> prev;
        std::shared_ptr<LRUNode<key_t, val_t>> next;

    public:
        LRUNode(key_t key, val_t val) : key(key), val(val), accessCount(0), prev(nullptr), next(nullptr) {}

        key_t getKey() { return key; }
        val_t getVal() { return val; }
        size_t getAccessCount() { return accessCount; }
        void setValue(const val_t &val) { this->val = val; }
        void incrementAccessCount() { accessCount++; }
        friend class LRUcache<key_t, val_t>;
    };

    template <typename key_t, typename val_t>
    class LRUcache : public CacheBase<key_t, val_t>
    {
    private:
        int capacity;
        node_map nodeMap;
        std::mutex mut;
        node_ptr dummyHead;
        node_ptr dummyTail;

    public:
        using node_t = LRUNode<key_t, val_t>;
        using node_ptr = std::shared_ptr<node_t>;
        using node_map = std::unordered_map<key_t, node_ptr>;
        LRUcache(size_t capacity) : capacity(capacity)
        {
            init();
        }
        ~LRUcache() override = default;

        void put(key_t key, val_t val) override
        {
            if (capacity <= 0)
            {
                return;
            }
        }
        void init();
    };
}
