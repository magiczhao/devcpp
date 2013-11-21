#ifndef _DEVCPP_BIGMAP_SEGMENT_H
#define _DEVCPP_BIGMAP_SEGMENT_H
#include "buffer.h"
namespace devcpp
{
namespace bigmap
{
/**
 * Segment对象表示可动态hash最顶级的目录，
 * 采用这个方法的动态hash可以充分利用文件的空间，具体原理参考：
 * http://citeseerx.ist.psu.edu/viewdoc/download?rep=rep1&type=pdf&doi=10.1.1.14.5908
 *
 * Segment的结构
 * 总容量(4B) | 已用容量(4B) | values(array of int)
 * Segment占用了一个Block(1M)，其中，前面部分为顶级目录
 * 其余部分为一个bitmap，保存了哪个hash项被使用了的信息
 */
class Segment
{
    public:
        static const int SEGMENT_COUNT = 14;
        static const int INIT_SLOT_COUNT = 4096;
        Segment(Buffer& buf) : _buf(buf)
        {
            _head = (int*) buf();
            _slots = _head + SEGMENT_COUNT;
        }
        ~Segment();
        
        Segment& set(Buffer& buf)
        {
            _buf = buf;
            _head = (int*) buf();
            _slots = _head + SEGMENT_COUNT;
            init();
            return *this;
        }

        int size() const 
        {
            return _head[1];
        }

        int capacity() const
        {
            return _head[0];
        }

        int extend()
        {
            return _head[1] += 1;
        }

        void init_slot(int number)
        {
            if(number < (INIT_SLOT_COUNT << (_head[1]))){
                _slots[number] = number;
            }
        }
        
        bool init();
        bool is_init(int number)
        {
            if(_slots[number] >= 0){
                return true;
            }
            return false;
        }

        int value(int number)
        {
            if(number < (INIT_SLOT_COUNT << _head[1])){
                return _slots[number];
            }
            return -1;
        }

    private:
        Buffer _buf;
        int* _head;
        int* _slots; 
};

}
}
#endif
