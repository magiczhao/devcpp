#ifndef _DEVLIB_SIMPLE_ALLOC_H
#define _DEVLIB_SIMPLE_ALLOC_H
/**
 * fixed size memory allocator, all blocks allocated are the same size
 */
#include "dlist.h"
#include <stdlib.h>

struct dmem_node
{
    struct dlist_node node;
};

struct fixed_dmem_pool
{
    /** free memory list */
    struct dlist_node head;
    /** max count of blocks can allocate */
    int item_size;
    /** memory size used by this allocator */
    int total_size;
    /** the memory area controled by this allocator */
    char* buffer;
    /** free area in the buffer */
    char* position;
};

/** 
 * if memory block and dmem_node share the same address 
 * so for item_size < sizeof(struct dmem_node), some memory will be wasted
 */
#define pool_block_size(pool)   (pool->item_size > sizeof(struct dmem_node) ?\
     pool->item_size : sizeof(struct dmem_node))

int fixed_pool_init(struct fixed_dmem_pool* pool, int capacity, int item_size);
void fixed_pool_fini(struct fixed_dmem_pool* pool);
void* d_fp_malloc(struct fixed_dmem_pool* pool, int size);
void d_fp_free(struct fixed_dmem_pool* pool, void* ptr);
#endif
