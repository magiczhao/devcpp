#include "block.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "trace.h"

namespace devcpp
{
namespace bigmap
{

BlockManager::~BlockManager()
{
    if(_file >= 0){
        close(_file);
        _file = -1;
    }
}

bool BlockManager::init(const string filename)
{
    if(_file >= 0){
        close(_file);
    }
    _file = open(filename.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);
    if(_file == -1){
        trace("open file failed:%s", strerror(errno));
        return false;
    }
    return true;
}

void BlockManager::free(Block* block)
{
    char* buffer = block->buffer();
    delete block;
    ::free(buffer);
}

bool BlockManager::save(Block* block)
{
    int offset = block->blockid() * BLOCK_SIZE + _start;
    if(pwrite(_file, block->buffer(), BLOCK_SIZE, offset) == BLOCK_SIZE){
        block->changed(false);
        return true;
    }
    return false;
}

Block* BlockManager::get(int blockid, int flag /*= 0x01*/)
{
    int offset = blockid * BLOCK_SIZE + _start;
    char* buffer = (char*)malloc(BLOCK_SIZE);
    if(!buffer){
        return NULL;
    }

    int bytes = pread(_file, buffer, BLOCK_SIZE, offset);
    if(bytes == BLOCK_SIZE){
        Block* blk = new Block(blockid, buffer, this);
        trace("bytes read:%d, blockid:%d", bytes, blockid);
        if(blk){
            return blk;
        }else{
            ::free(buffer);
        }
    }else if(bytes >= 0){
        trace("bytes read:%d, block_id:%d", bytes, blockid);
        Block* blk = new Block(blockid, buffer, this);
        blk->init(flag);
        return blk;
    }
    trace("bytes read:%d, %s, offset:%d, start:%d, block:%d", 
            bytes, strerror(errno), offset, _start, blockid);
    return NULL;
}

Block::~Block()
{
    trace("Block::~Block(), changed:%d", changed());
    save();
    _block = NULL;
}

bool Block::save()
{
    if(changed()){
        //trace("block save: flag:%d", _block->flag);
        return _manager->save(this);
    }else{
        return false;
    }
}

bool Block::init(int flag)
{
    //_block->flag = flag;
    if(block_is_dir(_block)){
        _block->body.dir.count = 0;
        _block->body.dir.remain = BLOCK_SIZE - sizeof(struct BlockHead);
        return true;
    }
    return false;
}

bool Block::remove(int position)
{
    trace("block begin id:%d, remove position:%d, size:%d, remain:%d", _blockid, position, size(), remain());
    if(_block->body.dir.count > position && position >= 0){
        int moved_size = _block->body.dir.items[position].size;
        int moved_start = _block->body.dir.items[position].start;
        for(int i = position; i < _block->body.dir.count - 1; ++i){
            //copy dir
            _block->body.dir.items[i] = _block->body.dir.items[i + 1];
            _block->body.dir.items[i].start += moved_size;
        }
        int last_start = _block->body.dir.items[_block->body.dir.count - 1].start;
        memmove(data_at(last_start + moved_size), 
            data_at(last_start), 
            moved_start - last_start);
        _block->body.dir.count -= 1;
        _block->body.dir.remain += moved_size + sizeof(struct BlockItem);
        changed(true);
    }
    trace("block begin id:%d, remove position:%d, size:%d, remain:%d", _blockid, position, size(), remain());
    return true;
}

int Block::insert(const char* value, int size)
{
    trace("inside insert, blockId:%d, size:%d, remain:%d", _blockid, this->size(), remain());
    if(block_is_dir(_block) && remain() > (size + sizeof(struct BlockItem))){
        int index = _block->body.dir.count;
        int start = BLOCK_SIZE - size;
        if(_block->body.dir.count > 0){
            start = _block->body.dir.items[index - 1].start - size;
        }
        //copy data
        memcpy(data_at(start), value, size);
        //reduce remain count
        _block->body.dir.remain -= size + sizeof(struct BlockItem);
        _block->body.dir.items[index].start = start;
        _block->body.dir.items[index].size = size;

        changed(true);
        _block->body.dir.count += 1;
        return index;
    }
    trace("insert to block, size:%d, block_is_dir:%d, remain:%d", size, block_is_dir(_block), remain());
    return -1;
}

int Block::find(const char* value, int size, compare_func compare)
{
    for(int i = 0; i < _block->body.dir.count; ++i){
        char* item_data = block_item_data(i);
        int item_size = block_item_size(i);
        if(compare(item_data, item_size, value, size) == 0){
            return i;
        }
    }

    return -1;
}

}
}
