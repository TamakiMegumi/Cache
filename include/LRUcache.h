#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
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
    private:
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

        void insertNode(node_ptr node);

        void evictLeastRecent();
    };
}
