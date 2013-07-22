#ifndef _DEVLIB_DBLOCK_LIST_H
#define _DEVLIB_DBLOCK_LIST_H

#include "common.h"
#include "dlist.h"
struct dblock_node
{
    size_t size;
    size_t capacity;
    char* data[0];
};

#define dblock_value_at(nd, idx) (idx) < (nd)->size ? (nd)->data[(idx)] : NULL

static inline int dblock_insert(struct dblock_node* node, char* value, int index)
{
    if(index <= node->size && index < node->capacity){
        memmove(&node->data[index + 1], &node->data[index], (node->size - index) * sizeof(char*));
        node->data[index] = value;
        return 0;
    }
    return -1;
}

static inline dblock_split_to(struct dblock_node* src, struct dblock_node* to, int count)
{
    if(count < src->size && count < to->capacity){
    }
}
#endif
