#ifndef DEVCPP_BIGMAP_SIMPLE_HASH_H
#define DEVCPP_BIGMAP_SIMPLE_HASH_H
#include <memory>
#include <assert.h>
#include <string>
#include "buffer.h"
#include "block.h"
#include "trace.h"
#include "maped_file.h"
using namespace std;

namespace devcpp
{
namespace bigmap
{

/**
 * Bucket的结构如下：
 * 数量[0]
 */
class Buckets
{
    public:
        Buckets(unsigned int* buffer, int size) : _buffer(buffer), _size(size)
        {}
        ~Buckets(){}

        int size() const {return 1 << _buffer[0];}
        void init()
        {
            _buffer[0] = 1;
            for(int i = 1; i < _size; ++i){
                _buffer[i] = -1;
            }

            for(int i = 1; i <= (1<<_buffer[0]); ++i){
                _buffer[i] = i - 1;
            }
        }

        bool extend()
        {
            int buckets = 1 << _buffer[0];
            if(buckets * 2 < _size){
                _buffer[0] += 1;
                return true;
            }
            return false;
        }

        unsigned int buddy(unsigned int bucket)
        {
            bucket += 1 << (_buffer[0] - 1);
            return bucket;
        }

        bool is_inited(unsigned int bucket) const 
        {
            if(bucket > 0){
                return _buffer[bucket] >= 0;
            }
            return false;
        }
        
        unsigned int find_init(unsigned int h)
        {
            for(unsigned int i = _buffer[0]; i > 0; --i){
                unsigned int bucket = h & ((1 << i) - 1);
                trace("bucket id:%d, i:%d, _buffer[0]:%d", bucket, i, _buffer[0]);
                if(is_inited(bucket + 1)){
                    return bucket + 1;
                }
            }
            trace("i not found for h[%u], size:%d", h, _buffer[0]);
            assert(false);
            return -1;
        }

        int hash(unsigned int h) const { return h & ((1 << _buffer[0]) - 1); }
    private:
        unsigned int* _buffer;
        int _size;
};

struct KVPair
{
    int key_size;
    int value_size;
    char buffer[0];
};

inline char* keyptr(struct KVPair& pair)
{
    return pair.buffer;
}

inline int keysize(struct KVPair& pair)
{
    return pair.key_size;
}

inline char* valueptr(struct KVPair& pair)
{
    return pair.buffer + pair.key_size;
}

inline int valuesize(struct KVPair& pair)
{
    return pair.value_size;
}

class StrHash
{
    public:
        static const int SEED = 1;
        unsigned int operator()(Buffer& value)
        {
            return (int) MurmurHashNeutral2(value(), value.size(), SEED);
        }

    private:
        unsigned int MurmurHashNeutral2 (const void * key, int len, unsigned int seed)
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

class KVEqual
{
    public:
        bool operator()(Buffer& data, Buffer& key)
        {
            struct KVPair* pair = (struct KVPair*)data();
            char* ptr = keyptr(*pair);
            int size = keysize(*pair);

            if(key.size() == size && strncmp(ptr, key(), size) == 0){
                return true;
            }
            return false;
        }
};

template<typename HashFunc, typename EqualFunc, int HeaderSize>
class SimpleHash
{
    public:
        SimpleHash(const string& filename, bool is_init) : _filename(filename), 
            _header(filename, 0, HeaderSize), _blk_mgr(HeaderSize), 
            _buckets((unsigned int*)_header.buffer(), HeaderSize / sizeof(unsigned int))
        {
            _blk_mgr.init(filename);
            if(is_init){
                _buckets.init();
            }
            _blk_mgr.init(filename);
        }

        ~SimpleHash()
        {}

        bool insert(Buffer& key, Buffer& value);
        bool search(Buffer& key, Buffer* value);
    private:
        bool copy_value(Buffer& buffer, Buffer* value);
        bool extend_block(int bucket);
        bool insert_into_block(int bucket, Buffer& data);
        string _filename;
        MappedFile _header;
        BlockManager _blk_mgr;
        Buckets _buckets;
        HashFunc _hash;
        EqualFunc _equal;
};

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::extend_block(int bucket)
{
    unsigned int target = _buckets.buddy(bucket);
    Block* blk = _blk_mgr.get(target);
    Block* old = _blk_mgr.get(bucket);
    BlockAutoPtr oldptr(old);
    BlockAutoPtr blkptr(blk);
    if(!blk){
        return false;
    }
    for(Block::Iterator it = old->begin(); it != old->end();){
        Buffer buf(it(), it.size());
        unsigned int h = _buckets.hash(_hash(buf));
        if(h == bucket){
            ++it;
        }else{
            if(blk->insert(it(), it.size()) == -1){
                return false;
            }
            old->remove(it.position());
        }
    }
    return true;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::insert_into_block(int bucket, Buffer& data)
{
    Block* blk = _blk_mgr.get(bucket);
    BlockAutoPtr ptr(blk);
    return blk->insert(data(), data.size());
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::insert(Buffer& key, Buffer& value)
{
    unsigned int bucket = _buckets.find_init(_hash(key));
    unsigned int total_size = sizeof(struct KVPair) + key.size() + value.size();
    char* total = new char[total_size];
    if(total){
        auto_ptr<char> ptr(total);
        struct KVPair* kv = (struct KVPair*)total; 
        kv->key_size = key.size();
        kv->value_size = value.size();

        memcpy(keyptr(*kv), key(), key.size());
        memcpy(valueptr(*kv), value(), value.size());
        Buffer data_in_block(total, total_size);
        int res = insert_into_block(bucket - 1, data_in_block);
        trace("about to insert into block:%d, res:%d", bucket - 1, res);
        if(res >= 0){
            trace("insert into block:%d", bucket - 1);
            return true;
        }else{
            trace("extending.... the bucket");
            if(extend_block(bucket - 1)){
                unsigned int newbucket = _buckets.find_init(_hash(key));
                trace("about to insert into block:%d", newbucket - 1);
                return insert_into_block(newbucket - 1, data_in_block);
            }
        }
    }
    return false;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::copy_value(Buffer& buffer, Buffer* value)
{
    char* b = (*value)();
    struct KVPair* kv = (struct KVPair*) buffer();
    if(valuesize(*kv) < value->size()){
        memcpy(b, valueptr(*kv), buffer.size());
        value->size(valuesize(*kv));
        return true;
    }
    return false;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::search(Buffer& key, Buffer* value)
{
    unsigned int bucket = _buckets.find_init(_hash(key));
    Block* blk = _blk_mgr.get(bucket - 1);
    BlockAutoPtr blkptr(blk);
    trace("search bucket id:%d, used:%d", bucket, blk->size());
    for(Block::Iterator it = (*blkptr)->begin(); it != (*blkptr)->end(); 
            ++it){
        Buffer buf(it(), it.size());
        if(_equal(buf, key)){
            return copy_value(buf, value);
        }
    }
    return false;
}

}
}
#endif
