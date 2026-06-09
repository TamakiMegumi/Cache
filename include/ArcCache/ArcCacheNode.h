#pragma once
#include <memory>
namespace CacheSpace
{
    template <typename Key, typename Value>
    class ArcNode
    {
    private:
        Key key;
        Value val;
        size_t accessCnt;
        std::weak_ptr<ArcNode> prev;
        std::shared_ptr<ArcNode> next;

    public:
        ArcNode() : accessCnt(1), next(nullptr) {}
        ArcNode(Key key, Value val)
            : key(key), value(val), ArcNode() {}
        Key getKey() const
        {
            return key;
        }
        Value getValue() const
        {
            return val;
        }
        size_t getAccessCnt() const
        {
            return accessCnt;
        }
        void setValue(const Value &value)
        {
            this->val = value;
        }
        void incrementAccessCnt()
        {
            accessCnt++;
        }
        template <typename K, typename V>
        friend class ArcLRUPart;
        template <typename K, typename V>
        friend class ArcLFUPart;
    };
}