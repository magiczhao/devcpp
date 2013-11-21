#include "log.h"

namespace devcpp
{
namespace bigmap
{
bool AppendOnlyLog::log(Buffer& key, Buffer& value)
{
    struct LogFormat header = {
        _sequence,  // seq
        key.size(), // key_size
        value.size() // value_size
    };
    struct iovec data[3];
    data[0].iov_base = &header;
    data[0].iov_len = sizeof(header);

    data[1].iov_base = key();
    data[1].iov_len = key.size();

    data[2].iov_base = value();
    data[2].iov_len = value.size();
    if(writev(_fd, data, 3) == log_item_size(&header)){
        return true;
    }else{
        return false;
    }
}

AppendOnlyLog::~AppendOnlyLog()
{
    if(_fd >= 0){
        close(_fd);
        _fd = -1;
    }
}
}
}
