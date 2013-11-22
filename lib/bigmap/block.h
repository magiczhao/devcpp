#ifndef _DEVCPP_LIB_BIGMAP_BLOCK_H
#define _DEVCPP_LIB_BIGMAP_BLOCK_H

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include "trace.h"
using namespace std;
namespace devcpp
{
namespace bigmap
{

/**
 * block structure:
 * block的大小为4k
 * 对1M的数据块，主要用来保存value，这些value的字段不需要进行查找
 * head section : 4(size) + 2(count) + array(direcotry)
 * head 是从0 往后写的，但是data是从后向前写的，一个block的中间部分是空的。
 */

#define BLOCK_SIZE  (1 << 20)   
#pragma pack(1)
/**
 * 文件头，保存了文件的元数据，主要用于校验和查看文件的整体信息：
 * 如：文件长度、版本号等
 */
struct FileHead
{
    int32_t version;
    int32_t magic;
    uint64_t size;
};

/**
 * 一个block尺寸的
 */
struct BlockItem
{
    int start;
    int size;
};

struct BlockDir
{
    uint32_t count;
    uint32_t remain;
    uint32_t level;
    struct BlockItem items[0];
};

struct BlockData
{
    uint32_t remain;
    char data[0];
};
/**
 * 每个block的开始都是一个BlockHead，保存了本block的信息
 */
struct BlockHead
{
    // flag的8个bit分别定义如下：
    // 0 - 是否是目录
    // 1 - 是否还有后续节点
    //char flag;
    union
    {
        struct BlockDir dir;
        struct BlockData data;
    } body;
};

#define block_is_dir(blk)  (true)
#define block_is_whole(blk) (false)
//#define block_is_dir(blk)  ((blk)->flag && 0x01)
//#define block_is_whole(blk) (((blk)->flag && 0x02) == 0)

#define block_dir_size(dir) (sizeof(struct BlockDir) + sizeof(struct BlockItem) * ((dir)->count))
#define block_dir_head_size(blk) (block_dir_size(&(blk->body.dir)))
#define block_level(blk) ((blk)->body.dir.level)

#pragma pack()

class Block;
class BlockManager
{
    public:
        BlockManager(int start) : _file(-1), _start(start)
        {}
        ~BlockManager();
        Block* get(int blockid, int flag = 0x01);
        bool save(Block* block);
        bool init(const string filename);
        void free(Block* block);
    private:
        int _file;
        int _start;
};

typedef int (*compare_func) (const char* v1, int size1, const char* v2, int size2);

class Block
{
    public:
        Block(int blockid, void* blk_buf, BlockManager* manager) : _blockid(blockid), 
            _block((struct BlockHead*) blk_buf), _manager(manager), _changed(false)
        {}
        ~Block();

        bool init(int flag);

        int insert(const char* value, int size);
        int find(const char* value, int size, compare_func compare);
        bool remove(int position);
        bool save();

        bool is_dir() const {return (block_is_dir(_block));}
        bool is_data() const {return !(block_is_dir(_block));}
        int level() const {return _block->body.dir.level;}
        int level(uint32_t val) {_block->body.dir.level = val; changed(true);}
        
        char* block_buffer()
        {
            if(is_data()){
                return _block->body.data.data;
            }
            return NULL;
        }

        int size() const
        {
            return _block->body.dir.count;
        }

        int block_has_item(int position) const
        {
            if(block_is_dir(_block)){
                return _block->body.dir.count > position;
            }
            return false;
        }

        char* block_item_data(int position)
        {
            struct BlockItem* item = &(_block->body.dir.items[position]);
            return data_at(0) + item->start;
        }

        int block_item_size(int position)
        {
            struct BlockItem* item = &(_block->body.dir.items[position]);
            return item->size;
        }

        int remain() const
        {
            if(block_is_dir(_block)){
                return _block->body.dir.remain;
            }else{
                return _block->body.data.remain;
            }
        }

        class Iterator
        {
            public:
                Iterator(Block* blk) : _blk(blk), _index(0)
                {}

                Iterator(Block* blk, int index) : _blk(blk), _index(index)
                {}

                Iterator(const Iterator& iter)
                {
                    _blk = iter._blk;
                    _index = iter._index;
                }

                ~Iterator()
                {}

                char* operator()()
                {
                    return _blk->block_item_data(_index);
                }

                int size()
                {
                    return _blk->block_item_size(_index);
                }

                int position() const
                {
                    return _index;
                }

                bool operator== (Iterator it)
                {
                    return it._index == _index && it._blk == _blk;
                }

                bool operator!= (Iterator it)
                {
                    return it._index != _index || it._blk != _blk;
                }

                Iterator& operator=(const Iterator& iter)
                {
                    _index = iter._index;
                    _blk = iter._blk;
                }

                Iterator& operator++()
                {
                    _index += 1;
                    return *this;
                }

                Iterator& operator--()
                {
                    _index -= 1;
                    return *this;
                }
            private:
                Block* _blk;
                int _index;
        };

        class ReverseIterator
        {
            public:
                ReverseIterator(Block* blk) : _blk(blk), _index(0)
                {}

                ReverseIterator(Block* blk, int index) : _blk(blk), _index(index)
                {}

                ReverseIterator(const ReverseIterator& iter)
                {
                    _blk = iter._blk;
                    _index = iter._index;
                }

                ~ReverseIterator()
                {}

                char* operator()()
                {
                    return _blk->block_item_data(_blk->size() - _index - 1);
                }

                int size()
                {
                    return _blk->block_item_size(_blk->size() - _index - 1);
                }

                int position() const
                {
                    return _blk->size() - _index - 1;
                }

                bool operator== (ReverseIterator it)
                {
                    return it._index == _index && it._blk == _blk;
                }

                bool operator!= (ReverseIterator it)
                {
                    return it._index != _index || it._blk != _blk;
                }

                ReverseIterator& operator=(const ReverseIterator& iter)
                {
                    _index = iter._index;
                    _blk = iter._blk;
                }

                ReverseIterator& operator++()
                {
                    _index += 1;
                    return *this;
                }
            private:
                Block* _blk;
                int _index;
        };

        Iterator begin() 
        {
            return Iterator(this, 0);
        }

        Iterator end()
        {
            return Iterator(this, _block->body.dir.count);
        }

        ReverseIterator rbegin()
        {
            return ReverseIterator(this, 0);
        }

        ReverseIterator rend()
        {
            return ReverseIterator(this, _block->body.dir.count);
        }

        int blockid() const {return _blockid;}
        char* buffer() {return data_at(0);}
        void changed(bool value) {_changed = value;}
        bool changed() const {return _changed;}
        BlockManager* manager() {return _manager;}
    private:
        int block_item_start(int position)
        {
            struct BlockItem* item = &(_block->body.dir.items[position]);
            return item->start;
        }

        char* data_at(int position) {return ((char*)_block) + position;}
        int _blockid;
        struct BlockHead* _block;
        BlockManager* _manager;
        bool _changed;
};

class BlockAutoPtr
{
    public:
        BlockAutoPtr(Block* blk) : _block(blk)
        {}

        ~BlockAutoPtr()
        {
            if(_block){
                _block->manager()->free(_block);
                _block = NULL;
            }
        }

        Block* operator*()
        {
            return _block;
        }
    private:
        Block* _block;
};
}
}
#endif
