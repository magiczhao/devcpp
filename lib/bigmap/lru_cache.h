#ifndef DEVCPP_BIGMAP_LRU_CACHE_H
#define DEVCPP_BIGMAP_LRU_CACHE_H

#include <ext/hash_map>
#include <string>
using namespace std;
using namespace __gnu_cxx;

#include "buffer.h"
#include "str_hash.h"

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
        string key;
    };
    struct ValueItem
    {
        struct ListNode* node;
        string value;
    };
    typedef hash_map<string, struct ValueItem, StrHash>  ValueMap;

    public:
        LruCache(int max) : _max(max)
        {
            _head.next = &_head;
            _head.prev = &_head;
        }
        ~LruCache()
        {
            _cache.clear();
            while(_head.next != &_head){
                struct ListNode* node = _head.next;
                remove_list(_head.next);
                delete node;
            }
        }

        void remove(string& key)
        {
            ValueMap::iterator it = _cache.find(key);
            if(it != _cache.end()){
                struct ValueItem& item = it->second;
                remove_list(item.node);
                delete item.node;
                _cache.erase(it);
            }
        }

        bool get(string& key, Buffer* value)
        {
            ValueMap::iterator it = _cache.find(key);
            if(it != _cache.end()){
                struct ValueItem& item = it->second;
                remove_list(item.node);
                insert_list(item.node);
                if(value->size() > item.value.size()){
                    memcpy((*value)(), item.value.c_str(), item.value.size());
                    value->size(item.value.size());
                    return true;
                }
            }
            return false;
        }

        void set(const string& key, const string& value)
        {
            ValueMap::iterator it = _cache.find(key);
            if(it != _cache.end()){
                ValueItem& item = it->second;
                item.value = value;
                remove_list(item.node);
                insert_list(item.node);
            }else{
                struct ListNode* node = new ListNode;
                node->key = key;
                struct ValueItem item = {node, value};
                _cache.insert(make_pair(key, item));
                ValueMap::iterator it = _cache.find(key);
                insert_list(node);
            }

            if(count() > _max){
                struct ListNode* last_one = last();
                remove(last_one->key);
            }
        }

        int count() const 
        {
            return _cache.size();
        }
    private:
        struct ListNode* last()
        {
            if(_head.prev != &_head){
                return _head.prev;
            }
            return NULL;
        }

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
            _head.next = node;
            node->prev = &_head; //install prev
        }

        ValueMap _cache;
        struct ListNode _head;
        int _max;
};

}
}
#endif
