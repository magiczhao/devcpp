#ifndef DEVCPP_BIGMAP_SIMPLE_HASH_H
#define DEVCPP_BIGMAP_SIMPLE_HASH_H
#include <memory>
#include <assert.h>
#include <string>
#include "buffer.h"
#include "block.h"
#include "trace.h"
#include "maped_file.h"
#include "lru_cache.h"
#include "str_hash.h"
using namespace std;

namespace devcpp
{
namespace bigmap
{

/**
 * Bucket的结构如下：
 * 当前采用的位，当前的block数量，每一个桶对应的blockid
 * 目前的设计里，一个block对应一个id
 */
class Buckets
{
    public:
        /**
         * buffer对应map进来的内存头部，保存整个目录信息的地方,结构如上所述
         * size 对应于内存的大小，对应到_size字段，需要减去前面所用到的
         */
        Buckets(unsigned int* buffer, int size) : _shift(buffer), _blocks(buffer + 1), 
            _buffer(buffer + 2), _size(size - 16)
        {
            trace("buckets init, shift:%d, %d", (int)_shift, (int)buffer);
        }
        ~Buckets(){}
        
        /** 
         * shift表示目前hash后使用的bit数量，bucket的数量就是2^shift()
         */
        unsigned int shift() const {return *_shift;}
        /**
         * size表示已经使用的bucket数量
         */
        unsigned int size() const {return 1 << (*_shift);}
        /**
         * blocks表示当前已经分配出去的block数量
         */
        unsigned int blocks() const {return *_blocks;}
        /**
         * 分配一个blockid
         */
        unsigned int occupy_block() 
        {
            unsigned int blockid = *_blocks;
            *_blocks += 1;
            return blockid;
        }

        void init()
        {
            trace("Buckets init size:%d", _size);
            *_shift = 1;
            for(int i = 0; i < _size; ++i){
                _buffer[i]= -1;
            }

            for(int i = 0; i <= (1<<(*_shift)); ++i){
                _buffer[i]= i;
            }
            *_blocks = 2;
        }

        /**
         * 把bucket的数量扩展一倍，新扩充的bucket指向原来hash & shift()的情况下对应的bucket
         * 如果之前的那个bucket满了再进行扩充
         */
        bool extend()
        {
            int buckets = 1 << (*_shift);
            if(buckets * 2 < _size){
                *_shift += 1;
                int scale = 1 << (*_shift);
                //trace("begin:%d, scale:%d", buckets, scale);
                for(int i = 0; i < buckets; ++i){ //初始化新的bucket
                    //trace("_buffer[%d]=%d, sibling:%d", i + buckets, _buffer[i], i);
                    _buffer[i + buckets] = _buffer[i];
                }
                return true;
            }
            return false;
        }
        
        /** 取得对应bucket的blockid */
        unsigned int bucket(int b) const {return _buffer[b];}
        /** 给一个bucket分配对应的blockid */
        void fill(int b) {_buffer[b] = occupy_block();}
        /** 计算hash值然后按照shift对应的取相应的bit出来 */
        int hash(unsigned int h) const { return h & ((1 << (*_shift)) - 1); }
    private:
        unsigned int* _shift;
        unsigned int* _blocks;
        unsigned int* _buffer;
        int _size;
};

/** 存储在block中的一个kv对 */
struct KVPair
{
    /** key的长度 */
    int key_size;
    /** value的长度 */ 
    int value_size;
    /** key value保存在这里 */
    char buffer[0];
};

/** 一些KVPair的工具函数 */
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


/** 字符串相等判断的类 */
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

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize = 10240>
class SimpleHash
{
    public:
        SimpleHash(const string& filename, bool is_init) : _filename(filename), 
            _header(filename, 0, HeaderSize), _blk_mgr(HeaderSize), 
            _buckets((unsigned int*)_header.buffer(), HeaderSize / sizeof(unsigned int)),
            _cache(CacheSize)
        {
            _blk_mgr.init(filename);
            if(is_init){
                _buckets.init();
                unsigned int shift = _buckets.shift();
                for(int i = 0; i < _buckets.blocks(); ++i){
                    Block* blk = _blk_mgr.get(i);
                    BlockAutoPtr ptr(blk);
                    blk->level(shift);
                }
            }
        }

        ~SimpleHash()
        {}
        /** 在hash表中插入一条记录 */
        bool insert(Buffer& key, Buffer& value);
        /** 在hash表中读取一条记录,结果保存在value中，这个value的size要设置成总共可用的内存大小 */
        bool search(Buffer& key, Buffer* value);
    private:
        /** 把KVPair中的value部分拷贝到value中 */
        bool copy_value(Buffer& buffer, Buffer* value);
        bool extend_block();
        bool insert_into_block(int bucket, Buffer& key, Buffer& value, Buffer& data);
        bool fill_bucket(int bucket);
        bool need_extend(int blockid)
        {
            Block* blk = _blk_mgr.get(blockid);
            BlockAutoPtr ptr(blk);
            trace("blk->level:%d, shift:%d", blk->level(), _buckets.shift());
            return blk->level() == _buckets.shift();
        }

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
        LruCache _cache;
};


template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::fill_bucket(int bucket)
{
    int blockid = _buckets.bucket(bucket);
    _buckets.fill(bucket);
    int nblockid = _buckets.bucket(bucket);
    trace("fill_bucket for bucket:%d, from :%d to:%d", bucket, blockid, nblockid);
    Block* target = _blk_mgr.get(nblockid);
    Block* old = _blk_mgr.get(blockid);
    BlockAutoPtr tgtptr(target);
    BlockAutoPtr oldptr(old);

    target->level(_buckets.shift());
    old->level(old->level() + 1);
    if(!target || !old){
        return false;
    }
    for(Block::Iterator it = old->begin(); it != old->end();){
        Buffer key;
        key_part(it(), it.size(), &key);
        //trace("key:%s, %d, %d, %d", key(), key.size(), (int)it(), it.size());
        unsigned int h = _buckets.hash(_hash(key));
        if(h == bucket){
            if(target->insert(it(), it.size()) == -1){
                return false;
            }
            old->remove(it.position());
        }else{
            ++it;
        }
    }
    return true;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::extend_block()
{
    return _buckets.extend();
}

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::insert_into_block(int bucket, Buffer& key, Buffer& value, Buffer& data)
{
    Block* blk = _blk_mgr.get(bucket);
    BlockAutoPtr ptr(blk);
    for(Block::Iterator it = blk->begin(); it != blk->end(); 
            ++it){
        Buffer buf(it(), it.size());
        if(_equal(buf, key)){
            blk->remove(it.position());
            break;
        }
    }
    bool res = (blk->insert(data(), data.size()) >= 0);
    if(res){
        string kstr(key(), key.size());
        string vstr(value(), value.size());
        _cache.set(kstr, vstr);
    }
    return res;
} 

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::insert(Buffer& key, Buffer& value)
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
        bool res = insert_into_block(blockid, key, value, data_in_block);
        //trace("about to insert into block:%d, res:%d", bucket, res);
        if(res){
            trace("insert into block:%d", bucket);
            return true;
        }else{
            if(!need_extend(blockid)){  //现在是借来的空间，分配实际空间
                trace("filling the bucket %d", blockid);
                fill_bucket(bucket);
                blockid = _buckets.bucket(bucket);
                return insert_into_block(blockid, key, value, data_in_block);
            }else{
                trace("extending.... the buckets");
                if(extend_block()){
                    unsigned int newbucket = _buckets.hash(_hash(key));
                    fill_bucket(newbucket);
                    blockid = _buckets.bucket(newbucket);
                    trace("about to insert into bucket:%d, block:%d", newbucket, blockid);
                    return insert_into_block(blockid, key, value, data_in_block);
                }
            }
        }
    }
    return false;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::copy_value(Buffer& buffer, Buffer* value)
{
    char* b = (*value)();
    struct KVPair* kv = (struct KVPair*) buffer();
    if(valuesize(*kv) < value->size()){
        memcpy(b, valueptr(*kv), valuesize(*kv));
        value->size(valuesize(*kv));
        return true;
    }
    return false;
}

template<typename HashFunc, typename EqualFunc, int HeaderSize, int CacheSize>
bool SimpleHash<HashFunc, EqualFunc, HeaderSize, CacheSize>::search(Buffer& key, Buffer* value)
{
    string kstr(key(), key.size());
    if(_cache.get(kstr, value)){
        return true;
    }
    unsigned int bucket = _buckets.hash(_hash(key));
    int blockid = _buckets.bucket(bucket);
    Block* blk = _blk_mgr.get(blockid);
    BlockAutoPtr blkptr(blk);
    trace("search bucket id:%d, block:%d, used:%d", bucket, blockid, blk->size());
    for(Block::Iterator it = blkptr()->begin(); it != blkptr()->end(); ++it){
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
