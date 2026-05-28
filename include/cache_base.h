#pragma once
namespace CacheSpace
{

    template <typename Key, typename Value>
    class CacheBase
    {
    protected:
    public:
        virtual ~CacheBase() = default;
        virtual bool get(Key key, Value &val) = 0;
        virtual void put(Key key, Value val) = 0;
        virtual Value get(Key key) = 0;
    };

}
