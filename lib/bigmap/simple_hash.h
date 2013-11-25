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
 * Bucket�Ľṹ���£�
 * ��ǰ���õ�λ����ǰ��block������ÿһ��Ͱ��Ӧ��blockid
 * Ŀǰ������һ��block��Ӧһ��id
 */
class Buckets
{
    public:
        /**
         * buffer��Ӧmap�������ڴ�ͷ������������Ŀ¼��Ϣ�ĵط�,�ṹ��������
         * size ��Ӧ���ڴ�Ĵ�С����Ӧ��_size�ֶΣ���Ҫ��ȥǰ�����õ���
         */
        Buckets(unsigned int* buffer, int size) : _shift(buffer), _blocks(buffer + 1), 
            _buffer(buffer + 2), _size(size - 16)
        {
            trace("buckets init, shift:%d, %d", (int)_shift, (int)buffer);
        }
        ~Buckets(){}
        
        /** 
         * shift��ʾĿǰhash��ʹ�õ�bit������bucket����������2^shift()
         */
        unsigned int shift() const {return *_shift;}
        /**
         * size��ʾ�Ѿ�ʹ�õ�bucket����
         */
        unsigned int size() const {return 1 << (*_shift);}
        /**
         * blocks��ʾ��ǰ�Ѿ������ȥ��block����
         */
        unsigned int blocks() const {return *_blocks;}
        /**
         * ����һ��blockid
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
         * ��bucket��������չһ�����������bucketָ��ԭ��hash & shift()������¶�Ӧ��bucket
         * ���֮ǰ���Ǹ�bucket�����ٽ�������
         */
        bool extend()
        {
            int buckets = 1 << (*_shift);
            if(buckets * 2 < _size){
                *_shift += 1;
                int scale = 1 << (*_shift);
                //trace("begin:%d, scale:%d", buckets, scale);
                for(int i = 0; i < buckets; ++i){ //��ʼ���µ�bucket
                    //trace("_buffer[%d]=%d, sibling:%d", i + buckets, _buffer[i], i);
                    _buffer[i + buckets] = _buffer[i];
                }
                return true;
            }
            return false;
        }
        
        /** ȡ�ö�Ӧbucket��blockid */
        unsigned int bucket(int b) const {return _buffer[b];}
        /** ��һ��bucket�����Ӧ��blockid */
        void fill(int b) {_buffer[b] = occupy_block();}
        /** ����hashֵȻ����shift��Ӧ��ȡ��Ӧ��bit���� */
        int hash(unsigned int h) const { return h & ((1 << (*_shift)) - 1); }
    private:
        unsigned int* _shift;
        unsigned int* _blocks;
        unsigned int* _buffer;
        int _size;
};

/** �洢��block�е�һ��kv�� */
struct KVPair
{
    /** key�ĳ��� */
    int key_size;
    /** value�ĳ��� */ 
    int value_size;
    /** key value���������� */
    char buffer[0];
};

/** һЩKVPair�Ĺ��ߺ��� */
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


/** �ַ�������жϵ��� */
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
        /** ��hash���в���һ����¼ */
        bool insert(Buffer& key, Buffer& value);
        /** ��hash���ж�ȡһ����¼,���������value�У����value��sizeҪ���ó��ܹ����õ��ڴ��С */
        bool search(Buffer& key, Buffer* value);
    private:
        /** ��KVPair�е�value���ֿ�����value�� */
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
            if(!need_extend(blockid)){  //�����ǽ����Ŀռ䣬����ʵ�ʿռ�
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
