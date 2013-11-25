#include "simple_hash.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

typedef devcpp::bigmap::StrHash Hash;
typedef devcpp::bigmap::KVEqual Equal;
typedef devcpp::bigmap::SimpleHash<Hash, Equal, 8 * 1024 * 1024, 10240> SHashTable;
static SHashTable* stable = NULL;
#ifdef __cplusplus
extern "C" {
#endif

int stable_init(const char* filename)
{
    struct stat fst;
    bool init = true;
    if(stat(filename, &fst) == 0){
        init = false;
    }else{
        if(errno != ENOENT){
            return -1;
        }
    }
    stable = new SHashTable(filename, init);
    return 0;
}

void stable_fini()
{
    delete stable;
    stable = NULL;
}

int stable_insert(char* key, char* value, int valuesize)
{
    devcpp::bigmap::Buffer kbuf(key, strlen(key));
    devcpp::bigmap::Buffer vbuf(value, valuesize);
    if(stable && stable->insert(kbuf, vbuf)){
        return 0;
    }else{
        return -1;
    }
}

int stable_search(char* key, char* value, int valuesize)
{
    devcpp::bigmap::Buffer kbuf(key, strlen(key));
    devcpp::bigmap::Buffer vbuf(value, valuesize);
    if(stable && stable->search(kbuf, &vbuf)){
        return vbuf.size();
    }else{
        return -1;
    }
}
#ifdef __cplusplus
}
#endif
