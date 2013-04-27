#include "dstring.h"
#include "common.h"

int kmp_search(const struct dstring* haystack, const struct dstring* needle)
{
    char* p = dstring_buffer(needle);
    char* h = dstring_buffer(haystack);
    int pptable[dstring_size(needle)];
    //build property table
    for(int i = 0; i < dstring_size(needle); ++i){
        if(i == 0){pptable[i] = 0; continue;}
        if(*(p + pptable[i - 1]) == *(p + i)){
            pptable[i] = pptable[i - 1] + 1;
        }else{
            pptable[i] = 0;
        }
    }
    //search here
    int pos = 0;
    while((h - dstring_buffer(haystack)) < dstring_size(haystack)){
        if(*h == *(p + pos)){
            if(pos + 1 == dstring_size(needle)){
                return h - dstring_buffer(haystack) - dstring_size(needle) + 1;
            }
            ++pos;
        }else{
            if(pos > 0) pos = pptable[pos - 1];
            else pos = 0;
        }
        ++h;
    }
    if(pos == dstring_size(needle))return pos - dstring_size(needle);
    return -1;
}
