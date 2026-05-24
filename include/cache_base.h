#pragma once
namespace Cache
{

    template <typename key_t, typename val_t>
    class CacheBase
    {
    protected:
    public:
        virtual ~CacheBase() = default;
        virtual bool get(key_t key, val_t &val) = 0;
        virtual void put(key_t key, val_t val) = 0;
        virtual val_t get(key_t key) = 0;
    };

}
