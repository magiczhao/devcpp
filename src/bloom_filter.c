#include "common.h"
#include "bloom_filter.h"
#include "bit.h"

int dbloom_filter_init(struct dbloom_filter* dbf, int capacity, int hash_count)
{
    dbf->capacity = capacity;
    dbf->func_count = hash_count < DHASH_FUNC_MAX_SIZE ? hash_count : DHASH_FUNC_MAX_SIZE;
    dbf->fail_count = 0;
    dbf->buffer = (char*) malloc(capacity >> 3);
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
    for(i = 0; i < dbf->func_count; ++i){
        unsigned int hval = dbf->hash[i](value, size);
        int byte = hval >> 3;
        int bit = hval - (byte << 3);
        set_bit((dbf->buffer + byte), bit);
    }
}

int dbloom_filter_get(struct dbloom_filter* dbf, const char* value, unsigned int size)
{
    for(int i = 0; i < dbf->func_count; ++i){
        unsigned int hval = dbf->hash[i](value, size);
        int byte = hval >> 3;
        int bit = hval -(byte << 3);
        if(!is_bit_set((dbf->buffer + byte), bit)){
            return false;
        }
    }
    return true;
}
