#ifndef DEVCPP_BIGMAP_LRU_CACHE_H
#define DEVCPP_BIGMAP_LRU_CACHE_H

#include <ext/hash_map>
using namespace __gnu_cxx;

#include "buffer.h"

namespace devcpp
{
namespace bigmap
{


class LruCache
{
    struct ListNode
    {
        struct ListNode* next;
        struct ListNode* prev;
        int block_id;
    };
    struct BlockItem
    {
        Block* blk;
        struct ListNode* node;
    };
    typedef hash_map<unsigned int, struct BlockItem>  BlockMap;

    public:
        LruCache(int max) : _max(max)
        {
            _head.next = &_head;
            _head.prev = &_head;
            _head.block_id = -1;
        }
        ~LruCache()
        {
            for(BlockMap::iterator it = _cache.begin(); it != _cache.end();){
                Block* blk = it->second.blk;
                blk->manager()->free(blk);  //free block memory
                remove_list(it->second.node); // remove from list
                delete it->second.node;  // free list node
                _cache.erase(it++); // remove from hash
            }
        }

        void remove(unsigned int blockid)
        {
            BlockMap::iterator it = _cache.find(blockid);
            if(it != _cache.end()){
                Block* blk = it->second.blk;
                blk->manager()->free(blk);
                remove_list(it->second.node);
                delete it->second.node;
                _cache.erase(it);
            }
        }

        Block* get(unsigned int blockid)
        {
            BlockMap::iterator it = _cache.find(blockid);
            if(it != _cache.end()){
                remove_list(it->second.node);
                insert_list(it->second.node);
                return it->second.blk;
            }
            return NULL;
        }

        void set(unsigned int blockid, Block* buf)
        {
            BlockMap::iterator it = _cache.find(blockid);
            if(it != _cache.end()){
                Block* blk = it->second.blk;
                blk->manager()->free(blk);
                remove_list(it->second.node);
                insert_list(it->second.node);
                it->second.blk = buf;
            }else{
                struct ListNode* node = new ListNode;
                node->block_id = blockid;
                struct BlockItem item = {buf, node};
                _cache.insert(make_pair(blockid, item));
            }
        }

        int count() const 
        {
            return _cache.size();
        }
    private:

        void remove_list(struct ListNode* node)
        {
            struct ListNode* prev = node->prev;
            struct ListNode* next = node->next;
            prev->next = next;
            next->prev = prev;
        }

        void insert_list(struct ListNode* node)
        {
            struct ListNode* next = _head.next;
            node->next = next;
            next->prev = node; //install next
            _head->next = node;
            node->prev = &_head; //install prev
        }

        BlockMap _cache;
        struct ListNode _head;
        int _max;
};

}
}
#endif
