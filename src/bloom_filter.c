#include "common.h"
#include "bloom_filter.h"
#include "bit.h"

int dbloom_filter_init(struct dbloom_filter* dbf, int capacity, int hash_count)
{
    memcpy(dbf->hash, hfuncs, sizeof(dbf->hash));
    dbf->capacity = capacity;
    dbf->func_count = hash_count < DHASH_FUNC_MAX_SIZE ? hash_count : DHASH_FUNC_MAX_SIZE;
    //printf("hcount:%d, %d, %d, %d\n", hash_count, DHASH_FUNC_MAX_SIZE, dbf->func_count, (capacity >> 3));
    dbf->fail_count = 0;
    dbf->buffer = (char*) malloc((capacity >> 3) + 1);
    if(!dbf->buffer){
        errno = ENOMEM;
        return -1;
    }
    return 0;
}

void dbloom_filter_fini(struct dbloom_filter* dbf)
{
    free(dbf->buffer);
    dbf->buffer = NULL;
}

void dbloom_filter_add(struct dbloom_filter* dbf, const char* value, unsigned int size)
{
    int i = 0;
    //printf("hash funcs:%d, %d\n", (int)dbf, (int)(dbf->buffer));
    for(i = 0; i < dbf->func_count; ++i){
        unsigned int hval = dbf->hash[i](value, size);
        unsigned int hpos = hval % (dbf->capacity);
        int byte = hpos >> 3;
        int bit = hpos - (byte << 3);
        //printf("byte:%d, bit:%d, %d\n", byte, bit, (int)(dbf->buffer + byte));
        //*(dbf->buffer + byte) = 1;
        set_bit((dbf->buffer + byte), bit);
    }
}

int dbloom_filter_find(struct dbloom_filter* dbf, const char* value, unsigned int size)
{
    for(int i = 0; i < dbf->func_count; ++i){
        unsigned int hval = dbf->hash[i](value, size);
        unsigned int hpos = hval % (dbf->capacity);
        int byte = hpos >> 3;
        int bit = hpos - (byte << 3);
        if(!is_bit_set((dbf->buffer + byte), bit)){
            return false;
        }
    }
    return true;
}
