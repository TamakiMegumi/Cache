#pragma once
#include "ArcCacheNode.h"
#include <unordered_map>
#include <mutex>

namespace CacheSpace
{
    template <typename Key, typename Value>
    class ArcLRUPart
    {
    public:
        using node_t = ArcNode<Key, Value>;
        using node_ptr = std::shared_ptr<node_t>;
        using node_map = std::unordered_map<Key, node_ptr>;

    private:
        size_t capacity;
        size_t ghostCapacity;
        size_t transformThreshold;
        std::mutex mtx;
        node_map mainCache;
        node_map ghostCache;

        node_ptr mainHead;
        node_ptr mainTail;
        node_ptr ghostHead;
        node_ptr ghostTail;

    public:
        explicit ArcLRUPart(size_t capacity, size_t transformThreshold)
            : capacity(capacity), ghostCapacity(capacity),
              transformThreshold(transformThreshold)
        {
            initializeLists();
        }
        bool put(Key key, Value val)
        {
            if (capacity == 0)
            {
                return false;
            }
            std::lock_guard<std::mutex> lock(mtx);
            auto it = mainCache.find(key);
            if (it != mainCache.end())
            {
                return updateExistingNode(it->second, val);
            }
            return addNewNode(key, val);
        }
        bool get(Key key, Value &val, bool &shouldTransform)
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = mainCache.find(key);
            if (it != mainCache.end())
            {
                shouldTransform = updateNodeAccess(it->second);
                val = it->second->getValue();
                return true;
            }
            return false;
        }
        bool checkGhost(Key key)
        {
            auto it = ghostCache.find(key);
            if (it != ghostCache.end())
            {
                removeFromGhost(it->second);
                ghostCache.erase(it);
                return true;
            }
        }
        void increaseCapacity()
        {
            ++capacity;
        }
        bool decreaseCapacity()
        {
            if (capacity <= 0)
            {
                return false;
            }
            if (mainCache.size() == capacity)
            {
                evictLeastRecent();
            }
            --capacity;
            return true;
        }

    private:
        void initializeLists()
        {
            mainHead = std::make_shared<node_t>();
            mainTail = std::make_shared<node_t>();
            mainHead->next = mainTail;
            mainTail->prev = mainHead;

            ghostHead = std::make_shared<node_t>();
            ghostTail = std::make_shared<node_t>();
            ghostHead->next = ghostTail;
            ghostTail->prev = ghostHead;
        }
        bool updateExistingNode(node_ptr node, const Value &val)
        {
            node->setValue(val);
            moveToFront(node);
            return true;
        }
        bool addNewNode(const Key &key, const Value &val)
        {
            if (mainCache.size() >= capacity)
            {
                evictLeastRecent();
            }
            node_ptr newNode = std::make_shared<node_t>(key, val);
            mainCache[key] = newNode;
            addToFront(newNode);
            return true;
        }
        bool updateNodeAccess(node_ptr node)
        {
            moveToFront(node);
            node->incrementAccessCnt();
            return node->getAccessCnt() >= transformThreshold;
        }
        void moveToFront(node_ptr node)
        {
            if (!node->prev.expired() && node->next)
            {
                auto prev = node->prev.lock();
                prev->next = node->next;
                node->next->prev = node->prev;
                node->next = nullptr;
            }
            addToFront(node);
        }
        void addToFront(node_ptr node)
        {
            node->next = mainHead->next;
            node->prev = mainHead;
            mainHead->next->prev = node;
            mainHead->next = node;
        }
        void evictLeastRecent()
        {
            node_ptr least = mainTail->prev.lock();
            if (!least || least == mainHead)
            {
                return;
            }
            removeFromMain(least);
            if (ghostCache.size() >= ghostCapacity)
            {
                removeOldestGhost();
            }
            addToGhost(least);
            mainCache.erase(least->getKey());
        }
        void removeFromMain(node_ptr node)
        {
            if (!node->prev.expired() && node->next)
            {
                auto prev = node->prev.lock();
                prev->next = node->next;
                node->next->prev = node->prev;
                node->next = nullptr;
            }
        }
        void removeFromGhost(node_ptr node)
        {
            if (!node->prev.expired() && node->next)
            {
                auto prev = node->prev.lock();
                prev->next = node->next;
                node->next->prev = node->prev;
                node->next = nullptr;
            }
        }
        void addToGhost(node_ptr node)
        {
            node->accessCnt = 1;
            node->next = ghostHead->next;
            node->prev = ghostHead;
            ghostHead->next->prev = node;
            ghostHead->next = node;
            ghostCache[node->getKey()] = node;
        }
        void removeOldestGhost()
        {
            node_ptr oldest = ghostTail->prev.lock();
            if (!oldest || oldest == ghostHead)
            {
                return;
            }
            removeFromGhost(oldest);
            ghostCache.erase(oldest->getKey());
        }
    };
}