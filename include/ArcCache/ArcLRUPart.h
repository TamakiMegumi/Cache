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
        bool put(Key key, Value val);
        bool get(Key key,Value& val,bool& shouldTransform);
        bool chechGhost(Key key);
        void increaseCapacity(){
            ++capacity;
        }
        bool decreaseCapacity();
    private:
        void initializeLists();
        bool updateExistingNode(node_ptr node, const Value &);
        bool addNewNode(const Key &key, const Value &val);
        bool updateNodeAccess(node_ptr node);
        void moveToFront(node_ptr node);
        void addToFront(node_ptr node);
        void evictLeastRecent();
        void removeFrmoMain(node_ptr node);
        void removeFromGhost(node_ptr node);
        void addToGhost(node_ptr node);
        void removeOldestGhost();
    };
}