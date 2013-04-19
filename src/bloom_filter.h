#ifndef _DEVLIB_BLOOM_FILTER_H
#define _DEVLIB_BLOOM_FILTER_H
/**
 * simple implementation of bloom filter
 * the theory is http://billmill.org/bloomfilter-tutorial/
 */
#include "hash_func.h"
#include "common.h"
static dhash_func hfuncs[] = {
    RSHash, JSHash, PJWHash, ELFHash, BKDRHash,
    SDBMHash, DJBHash, DEKHash, BPHash, FNVHash, 
    APHash
};

#define DHASH_FUNC_MAX_SIZE array_size(hfuncs)
struct dbloom_filter
{
    dhash_func hash[DHASH_FUNC_MAX_SIZE];
    int capacity;
    int func_count;
    unsigned int fail_count;
    char* buffer;
};

/** counter bloom filter fail, to caculate fail rate */
#define mark_fail(bf)   do{(bf)->fail_count += 1;}while(0)

/** init a bloom filter */
int dbloom_filter_init(struct dbloom_filter*, int capacity, int hash_count);
void dbloom_filter_fini(struct dbloom_filter*);
void dbloom_filter_add(struct dbloom_filter*, const char* value, unsigned int size);
int dbloom_filter_find(struct dbloom_filter*, const char* value, unsigned int size);
float get_fail_rate(struct dbloom_filter*);
#endif
