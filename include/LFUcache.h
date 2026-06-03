#pragma once
#include <memory>
#include "cache_base.h"
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
    protected:
    public:
    };
}
