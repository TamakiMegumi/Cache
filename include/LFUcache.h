#pragma once
#include <memory>
#include "cache_base.h"
#include <unordered_map>
#include <mutex>
namespace CacheSpace
{
    template <typename Key, typename Value>
    class LFUcache;
    template <typename Key, typename Value>
    class FreqList
    {
    private:
        struct Node
        {
            int freq;
            Key key;
            Value value;
            std::weak_ptr<Node> pre;
            std::shared_ptr<Node> next;
            Node() : freq(1), next(nullptr) {}
            Node(Key key, Value val)
                : freq(1), key(key), value(val), next(nullptr) {}
            using node_ptr = std::shared_ptr<Node>;
            int freq;
            node_ptr head, tail;

        public:
            explicit FreqList(int freq) : freq(freq)
            {
                head = std::make_shared<Node>();
                tail = std::make_shared<Node>();
                head->next = tail;
                tail->pre = head;
            }
            bool isEmpty() const
            {
                return head->next == tail;
            }
            void add(node_ptr node)
            {
                if (!node || !head || !tail)
                {
                    return;
                }
                node->pre = head;
                node->next = tail;
                tail->pre.lock()->next = node;
                tail->pre = node;
            }
            void remove(node_ptr node)
            {
                if (!node || !head || !tail)
                {
                    return;
                }
                if (node->pre.expired() || !node->next)
                {
                    return;
                }
                auto pre = node->pre.lock();
                pre->next = node->next;
                node->next->pre = pre;
                node->next = nullptr;
            }
            node_ptr getFirst() const
            {
                return head->next;
            }
            friend class LFUcache<Key, Value>;
        };
    };

    template <typename Key, typename Value>
    class LFUcache : public CacheBase<Key, Value>
    {
    public:
        using Node = FreqList<Key, Value>::Node;
        using node_ptr = std::shared_ptr<Node>;
        using node_map = std::unordered_map<Key, node_ptr>;

    private:
        void putInternal(Key key, Value val)
        {
            if (nodeMap.size() >= capacity)
            {
                kickOut();
            }
            node_ptr node = std::make_shared<Node>(key, val);
            nodeMap[key] = node;
            addToFreqList(node);
            addFreqNum();
            minfreq = std::min(minfreq, 1);
        }
        void getInternal(node_ptr node, Value &val)
        {
            val = node->value;
            removeFromFreqList(node);
            addToFreqList(node);
            if (node->freq - 1 == minfreq &&
                freqToFreqList[node->freq - 1]->isEmpty())
            {
                minfreq++;
            }
            addFreqNum();
        }
        void kickOut()
        {
            node_ptr node = freqToList[minfreq]->getFirst();
            removeFromFreqList(node);
            nodeMap.erase(node->key);
            decreaseFreqNum(node->freq);
        }
        void removeFromFreqList(node_ptr node)
        {
            if (!node)
            {
                return;
            }
            auto freq = node->freq;
            freqToList[freq]->remove(node);
        }
        void addToFreqList(node_ptr node)
        {
            if (!node)
            {
                return;
            }
            auto freq = node->freq;
            if (freqToList.find(freq) == freqToList.end())
            {
                freqToList[freq] = new FreqList<Key, Value>(freq);
            }
            freqToList[freq]->add(node);
        }
        void addFreqNum()
        {
            curSum++;
            if (nodeMap.empty())
            {
                curAvg = 0;
            }
            else
            {
                curAvg = curSum / nodeMap.size();
            }
            if (curAvg > maxAvg)
            {
                handleOverMaxAvg();
            }
        }
        void decreaseFreqNum(int num)
        {
            curSum -= num;
            if (nodeMap.empty())
            {
                curAvg = 0;
            }
            else
            {
                curAvg = curSum / nodeMap.size();
            }
        }
        void handleOverMaxAvg()
        {
            if (nodeMap.empty())
            {
                return;
            }
            for (auto it = nodeMap.begin(); it != nodeMap.end(); it++)
            {
                if (!it->second)
                {
                    continue;
                }
                node_ptr node = it->second;
                removeFromFreqList(node);
                int oldFreq = node->freq;
                int decay = maxAvg / 2;
                node->freq -= decay;
                if (node->freq <= 0)
                {
                    node->freq = 1;
                }
                curSum += node->freq - oldFreq;
                addToFreqList(node);
            }
            updateMinFreq();
        }
        void updateMinFreq()
        {
            minfreq = INT8_MAX;
            for (const auto &p : freqToList)
            {
                if (p.second && !p.second->isEmpty())
                {
                    minfreq = min(minfreq, p.first);
                }
            }
            if (minfreq == INT8_MAX)
            {
                minfreq = 1;
            }
        }

        int capacity;
        int minfreq;
        int maxAvg;
        int curAvg;
        int curSum;
        std::mutex mtx;
        node_map nodeMap;
        std::unordered_map<int, FreqList<Key, Value> *> freqToList;

    public:
        LFUcache(int cap, int maxAvg = 10)
            : capacity(cap), minfreq(INT8_MAX), maxAvg(maxAvg),
              curAvg(0), curSum(0) {}
        ~LFUcache() override
        {
            purge();
        }
        void put(Key key, Value val) override
        {
            if (capacity == 0)
            {
                return;
            }
            std::lock_guard<std::mutex> lock(mtx);
            auto it = nodeMap.find(key);
            if (it != nodeMap.end())
            {
                it->second->value = val;
                getInternal(key, val);
                return;
            }
            putInternal(key, val);
        }
        bool get(Key key, Value &val) override
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = nodeMap.find(key);
            if (it == nodeMap.end())
            {
                return false;
            }
            val = it->second->value;
            getInternal(key, val);
            return true;
        }
        Value get(Key key) override
        {
            Value val;
            get(key, val);
            return val;
        }
        void purge()
        {
            for (auto &p : freqToList)
            {
                delete p.second;
            }
            nodeMap.clear();
            freqToList.clear();
        }
    };

}
