#ifndef DEVCPP_BIGMAP_STR_HASH_H
#define DEVCPP_BIGMAP_STR_HASH_H

#include "buffer.h"
#include <string>
namespace devcpp
{
namespace bigmap
{
/** ×Ö·û´®hashµÄÀà */
class StrHash
{
    public:
        static const int SEED = 1;
        unsigned int operator()(Buffer& value)
        {
            return MurmurHashNeutral2(value(), value.size(), SEED);
        }
        size_t operator()(const std::string& value) const
        {
            return (size_t)MurmurHashNeutral2(value.c_str(), value.size(), SEED);
        }

    private:
        unsigned int MurmurHashNeutral2 (const void * key, int len, unsigned int seed) const
        {
            const unsigned int m = 0x5bd1e995;
            const int r = 24;

            unsigned int h = seed ^ len;

            const unsigned char * data = (const unsigned char *)key;

            while(len >= 4)
            {
                unsigned int k;

                k  = data[0];
                k |= data[1] << 8;
                k |= data[2] << 16;
                k |= data[3] << 24;

                k *= m; 
                k ^= k >> r; 
                k *= m;

                h *= m;
                h ^= k;

                data += 4;
                len -= 4;
            }

            switch(len)
            {
                case 3: h ^= data[2] << 16;
                case 2: h ^= data[1] << 8;
                case 1: h ^= data[0];
                        h *= m;
            };

            h ^= h >> 13;
            h *= m;
            h ^= h >> 15;

            return h;
        } 
};
}
}
#endif
