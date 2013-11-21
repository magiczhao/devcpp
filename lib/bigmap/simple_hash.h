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
        Buckets(unsigned int* buffer, int size) : _shift(buffer), _buffer(buffer + 1), _size(size - 1)
        {}
        ~Buckets(){}

        int size() const {return 1 << (*_shift);}
        void init()
        {
            trace("Buckets init size:%d", _size);
            *_shift = 1;
            for(int i = 0; i < _size; ++i){
                _buffer[i] = -1;
            }

            for(int i = 0; i <= (1<<(*_shift)); ++i){
                _buffer[i] = i;
            }
        }

        bool extend()
        {
            int buckets = 1 << (*_shift);
            if(buckets * 2 < _size){
                *_shift += 1;
                int scale = 1 << (*_shift);
                for(int i = buckets; i < scale; ++i){
                    _buffer[i] = (i >> 1);
                }
                return true;
            }
            return false;
        }

        int bucket(int b) const {return _buffer[b];}
        int fill(int b) {_buffer[b] = b;}

        /*
        unsigned int buddy(unsigned int bucket)
        {
            int target = bucket + (1 << (_buffer[0] - 1));
            if(is_inited(target)){
                if(extend()){
                    target = bucket + (1 << (_buffer[0] - 1));
                }
                trace("target %d ,%d already inited, extend to %d", target, _buffer[target], _buffer[0]);
            }
            return target;
        }

        bool is_inited(unsigned int bucket) const 
        {
            if(bucket < _size){
                return _buffer[bucket] == bucket - 1;
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

        */
        int hash(unsigned int h) const { return h & ((1 << (*_shift)) - 1); }
    private:
        unsigned int* _shift;
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
        bool extend_block();
        bool insert_into_block(int bucket, Buffer& data);
        bool fill_bucket(int bucket);

        void key_part(char* buffer, int size, Buffer* buf)
        {
            struct KVPair* kv = (struct KVPair*) buffer;
            int ks = keysize(*kv);
            *buf = keyptr(*kv);
            buf->size(ks);
        }
        string _filename;
        MappedFile _header;
        BlockManager _blk_mgr;
        Buckets _buckets;
        HashFunc _hash;
        EqualFunc _equal;
};


template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::fill_bucket(int bucket)
{
    int blockid = _buckets.bucket(bucket);
    _buckets.fill(bucket);
    int nblockid = _buckets.bucket(bucket);
    Block* target = _blk_mgr.get(nblockid);
    Block* old = _blk_mgr.get(blockid);
    BlockAutoPtr tgtptr(target);
    BlockAutoPtr oldptr(old);
    if(!target || !old){
        return false;
    }
    for(Block::Iterator it = old->begin(); it != old->end();){
        Buffer key;
        key_part(it(), it.size(), &key);
        unsigned int h = _buckets.hash(_hash(key));
        if(h == bucket){
            if(target->insert(it(), it.size()) == -1){
                return false;
            }
            old->remove(it.position());
        }
    }
    return true;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::extend_block()
{
    return _buckets.extend();
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::insert_into_block(int bucket, Buffer& data)
{
    Block* blk = _blk_mgr.get(bucket);
    BlockAutoPtr ptr(blk);
    return (blk->insert(data(), data.size()) >= 0);
}

template<typename HashFunc, typename EqualFunc, int HeaderSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize>::insert(Buffer& key, Buffer& value)
{
    unsigned int bucket = _buckets.hash(_hash(key));
    int blockid = _buckets.bucket(bucket);
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
        bool res = insert_into_block(blockid, data_in_block);
        trace("about to insert into block:%d, res:%d", bucket - 1, res);
        if(res){
            trace("insert into block:%d", bucket - 1);
            return true;
        }else{
            if(bucket != blockid){  //现在是借来的空间，分配实际空间
                trace("filling the bucket %d", blockid);
                fill_bucket(bucket);
                blockid = _buckets.bucket(bucket);
                return insert_into_block(blockid, data_in_block);
            }else{
                trace("extending.... the buckets");
                if(extend_block()){
                    unsigned int newbucket = _buckets.hash(_hash(key));
                    fill_bucket(newbucket);
                    blockid = _buckets.bucket(newbucket);
                    trace("about to insert into bucket:%d, block:%d", newbucket, blockid);
                    return insert_into_block(blockid, data_in_block);
                }
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
    unsigned int bucket = _buckets.hash(_hash(key));
    int blockid = _buckets.bucket(bucket);
    Block* blk = _blk_mgr.get(blockid);
    BlockAutoPtr blkptr(blk);
    trace("search bucket id:%d, block:%d, used:%d", bucket, blockid, blk->size());
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
