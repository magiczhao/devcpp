#ifndef _DEVLIB_SIMPLE_ALLOC_H
#define _DEVLIB_SIMPLE_ALLOC_H
#include "dlist.h"
#include <stdlib.h>

struct dmem_node
{
    struct dlist_node node;
};

struct fixed_dmem_pool
{
    struct dlist_node head;
    int item_size;
    int total_size;
    char* buffer;
    char* position;
};

/** 
 * if memory block and dmem_node share the same address 
 * so for item_size < sizeof(struct dmem_node), some memory will be wasted
 */
#define pool_block_size(pool)   (pool->item_size > sizeof(struct dmem_node) ?\
     pool->item_size : sizeof(struct dmem_node))

inline static int fixed_pool_init(struct fixed_dmem_pool* pool, int capacity, int item_size)
{
    INIT_DLIST_HEAD(&(pool->head));
    pool->item_size = item_size;
    pool->total_size = pool_block_size(pool) * capacity;
    pool->buffer = (char*)malloc(pool->total_size);
    pool->position = pool->buffer;
    if(pool->buffer){
        return 0;
    }
    return -1;
}

inline static void fixed_pool_fini(struct fixed_dmem_pool* pool)
{
    INIT_DLIST_HEAD(&(pool->head));
    free(pool->buffer);
    pool->buffer = NULL;
    pool->position = NULL;
    return;
}

inline static void* __d_fp_malloc_from_list(struct fixed_dmem_pool* pool)
{
    if(!DLIST_EMPTY(&pool->head)){
        struct dlist_node* node = pool->head.next;
        dlist_del(node);
        return container_of(node, struct dmem_node, node);
    }
    return NULL;
}

void* d_fp_malloc(struct fixed_dmem_pool* pool, int size)
{
#define FIXED_POOL_EMPTY(pl)  ((pl->position - pl->buffer) >= pl->total_size)
    if(pool->item_size > size){
        void * buf = __d_fp_malloc_from_list(pool);
        if(buf){
            return buf;
        }
        if(!FIXED_POOL_EMPTY(pool)){
            buf = pool->position;
            pool->position += pool_block_size(pool);
            return buf;
        }
    }
    return NULL;
#undef FIXED_POOL_EMPTY
}

void* d_fp_free(struct fixed_dmem_pool* pool, void* ptr)
{
    struct dmem_node* node = (struct dmem_node*) ptr;
    dlist_add(&pool->head, &node->node);
}
#endif
