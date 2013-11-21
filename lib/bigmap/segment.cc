#include "segment.h"
#include <stdio.h>

namespace devcpp
{
namespace bigmap
{

Segment::~Segment()
{}

bool Segment::init()
{
    for(int i = 0; i < SEGMENT_COUNT; ++i){
        _head[i] = -1;
    }
    _head[1] = 0;
    int slot_count = (_buf.size() / sizeof(int)) - SEGMENT_COUNT;
    for(int i = 0; i < slot_count; ++i){
        _slots[i] = -1; // init all slot to uninited
    }

    int init_count = slot_count < INIT_SLOT_COUNT ? slot_count : INIT_SLOT_COUNT;
    for(int i = 0; i < init_count; ++i){
        _slots[i] = i; //初始化slot也是从0开始
    }
    return true;
}

}
}
