#ifndef DEVCPP_BIGMAP_LOG_H
#define DEVCPP_BIGMAP_LOG_H

#include <string>
#include <assert.h>
using namespace std;

#include "buffer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>

namespace devcpp
{
namespace bigmap
{

struct LogFormat
{
    int seq;
    int key_length;
    int value_length;
    char buffer[0];
};

#define log_item_size(item) ((item)->key_length + (item)->value_length + sizeof(struct LogFormat))

class AppendOnlyLog
{
    public:
        AppendOnlyLog(const string& filename) : _filename(filename), _fd(-1), _sequence(1)
        {
            _fd = open(filename.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IRWXU);
            assert(_fd >= 0);
        }
        ~AppendOnlyLog();
        bool log(Buffer& key, Buffer& value);
    private:
        string _filename;
        int _fd;
        int _sequence;
};


}
}
#endif
