#pragma once

#include "ArcCacheNode.h"
#include <unordered_map>
#include <map>
#include <mutex>
#include <list>
namespace CacheSpace
{
    template <typename Key, typename Value>
    class ArcLFUPart
    {
    public:
        using node_t = ArcNode<Key, Value>;
        using node_ptr = std::shared_ptr<node_t>;
        using node_map = std::unordered_map<Key, node_ptr>;
        using freq_map = std::map<size_t, std::list<node_ptr>>;

    private:
        size_t capacity;
        size_t ghostCapacity;
        size_t transformThreshold;
        size_t minFreq;
        std::mutex mtx;

        node_map mainCache;
        node_map ghostCache;
        freq_map freqMap;
        node_ptr ghostHead;
        node_ptr ghostTail;

    public:
        explicit ArcLFUPart(size_t capacity, size_t transformThershold);
        bool put(Key key, Value val);
        bool get(Key key, Value &val);
        bool contain(Key key);
        bool checkGhost(Key key);
        void increaseCapacity();
        bool decreaseCapacity();

    private:
        void initializeLists();
        bool updateExistingNode(node_ptr node, const Value &val);
        bool addNewNode(const Key &key, const Value &val);
        void updateNodeFrequency(node_ptr node);
        void evictLeastFrequent();
        void removeFromGhost(node_ptr node);
        void addToGhost(node_ptr node);
        void removeOldestGhost();
    };
}