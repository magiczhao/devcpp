#ifndef _DEVCPP_LIB_BIGMAP_EQUAL_H
#define _DEVCPP_LIB_BIGMAP_EQUAL_H
class EqualBase
{
    public:

};

template<typename T>
class Equal : public EqualBase
{
    public:
        bool operator()(const T& v1, const T& v2)
        {
            return v1 == v2;
        }
};

template< >
class Equal<char*> : public EqualBase
{
    public:
        bool operator()(const char* s1, const char* s2)
        {
            return strncmp(s1, s2) == 0;
        }
};

#endif
