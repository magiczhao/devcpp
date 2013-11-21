#ifndef _DEVCPP_BIGMAP_SEGMENT_H
#define _DEVCPP_BIGMAP_SEGMENT_H
#include "buffer.h"
namespace devcpp
{
namespace bigmap
{
/**
 * Segment�����ʾ�ɶ�̬hash�����Ŀ¼��
 * ������������Ķ�̬hash���Գ�������ļ��Ŀռ䣬����ԭ��ο���
 * http://citeseerx.ist.psu.edu/viewdoc/download?rep=rep1&type=pdf&doi=10.1.1.14.5908
 *
 * Segment�Ľṹ
 * ������(4B) | ��������(4B) | values(array of int)
 * Segmentռ����һ��Block(1M)�����У�ǰ�沿��Ϊ����Ŀ¼
 * ���ಿ��Ϊһ��bitmap���������ĸ�hash�ʹ���˵���Ϣ
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
