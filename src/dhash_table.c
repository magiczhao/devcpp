#include "dhash_table.h"

#define EXPECT_RATE 16
static inline struct dhash_node* _dhash_find(struct dhash_table* ht, char* key, int key_size)
{
    unsigned int hval = ht->hash(key, key_size);
    unsigned int hpos = hval % ht->capacity;
    struct dlist_node* pos;
    dlist_for_each(pos, &(ht->buckets[hpos])){
        struct dhash_node* node = container_of(pos, struct dhash_node, lnode);
        if(ht->compare(dstring_buffer(&node->key), dstring_size(&node->key), key, key_size) == 0){ 
            return node;
        }
    }
    return NULL;
   
}

int dhash_table_init(struct dhash_table* ht, int capacity, 
    dhash_func hash, compare_func comp)
{
    ht->capacity = capacity;
    ht->hash = hash;
    ht->compare = comp;
    ht->buckets = (struct dlist_node*) malloc(sizeof(struct dlist_node) * capacity);
    int fret = fixed_pool_init(&ht->allocator, capacity * EXPECT_RATE, sizeof(struct dhash_node));
    if(fret == -1){
        return -1;
    }
    if(ht->buckets){
        for(int i = 0; i < capacity; ++i){
            INIT_DLIST_HEAD(&(ht->buckets[i]));
        }
        return 0;
    }
    fixed_pool_fini(&ht->allocator);
    return -1;
}

void dhash_table_fini(struct dhash_table* ht)
{
    fixed_pool_fini(&ht->allocator);
    free(ht->buckets);
    ht->buckets = NULL;
    return;
}

int dhash_table_add(struct dhash_table* ht, char* key, int key_size, void* value)
{
    if(_dhash_find(ht, key, key_size) == NULL){
        unsigned int hval = ht->hash(key, key_size);
        unsigned int hpos = hval % ht->capacity;
        struct dhash_node* node = (struct dhash_node*)d_fp_malloc(&ht->allocator, sizeof(struct dhash_node));
        if(node){
            dstring_assign((&node->key), key, key_size);
            node->value = value;
            dlist_add(&(ht->buckets[hpos]), &node->lnode);
        }else{
            errno = ENOMEM;
            return -1;
        }
    }else{
        errno = EEXIST;
        return -1;
    }
    return 0;
}


int dhash_table_find(struct dhash_table* ht, char* key, int key_size, void** value)
{
    struct dhash_node* node = _dhash_find(ht, key, key_size);
    if(node){
        *value = node->value;
        return true;
    }
    return false;
}

void* dhash_table_remove(struct dhash_table* ht, char* key, int key_size, char** orig_key)
{
    struct dhash_node* node = _dhash_find(ht, key, key_size);
    if(node){
        void * value = node->value;
        if(orig_key){
            *orig_key = dstring_buffer(&node->key);
        }
        dlist_del(&node->lnode);
        d_fp_free(&ht->allocator, node);
        return value;
    }
    return NULL;
}
