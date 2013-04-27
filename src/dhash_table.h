#ifndef _DEVLIB_HASH_TABLE_H
#define _DEVLIB_HASH_TABLE_H
#include "hash_func.h"
#include "dlist.h"
#include "common.h"
#include "fixed_alloc.h"
#include "dstring.h"
/**
 * a simple hash table implementation
 * fixed size allocator is used
 * key & values are stored inside hash table, and the caller must free them after remove from hash table
 */
 
/**
 * compare if two keys are same
 */
typedef int (*compare_func)(char*, int, char*, int);
struct dhash_node
{
    struct dlist_node lnode; //link on collision
    struct dstring key;
    void* value;
};

//TODO the table now is fixed size
struct dhash_table
{
    int capacity;
    dhash_func hash;
    compare_func compare;
    struct dlist_node* buckets;
    struct fixed_dmem_pool allocator;
};

int dhash_table_init(struct dhash_table* ht, int buckets, dhash_func hash, compare_func comp);
void dhash_table_fini(struct dhash_table* ht);

int dhash_table_add(struct dhash_table*, char* key, int key_size, void* value);
int dhash_table_find(struct dhash_table*, char* key, int key_size, void** value);
void* dhash_table_remove(struct dhash_table*, char* key, int key_size, char** orig_key);
#endif
