#include "simple_hash.h"
#include "buffer.h"
#include <gtest/gtest.h>

using namespace devcpp::bigmap;
typedef devcpp::bigmap::StrHash Hash;
typedef devcpp::bigmap::KVEqual Equal;
typedef devcpp::bigmap::SimpleHash<Hash, Equal, 8 * 1024 * 1024> SHashTable;
TEST(SimpleHash, InsertAndSearch)
{
    char buffer[16 * 1024];
    char key[128];
    snprintf(key, sizeof(key), "/root/1000.dat");
    snprintf(buffer, sizeof(buffer), "this is a test data");
    SHashTable stable("./tmp_simple_test.dat", true);
    char value[16 * 1024];
    Buffer kbuf(key, strlen(key));
    Buffer vbuf(value, sizeof(value));
    Buffer orig(buffer, strlen(buffer));
    ASSERT_TRUE(stable.insert(kbuf, orig));
    EXPECT_EQ(stable.search(kbuf, &vbuf), true);
    printf("%d:%s\n", vbuf.size(), vbuf());
    EXPECT_EQ(strncmp(vbuf(), orig(), orig.size()), 0);
}

TEST(SimpleHash, MultiInsert)
{
    char buffer[16 * 1024];
    char key[128];
    char value[16 * 1024];
    SHashTable stable("./tmp_multi_test.dat", true);
    for(int i = 0; i < 1024; ++i){
        snprintf(key, sizeof(key), "/root/%d.dat.value", i);
        snprintf(buffer, sizeof(buffer), "this is a value part of key %d\n", i);
        Buffer kbuf(key, strlen(key));
        Buffer vbuf(value, sizeof(value));
        Buffer orig(buffer, sizeof(buffer) - 1);
        printf("init value:%d\n", i);
        ASSERT_TRUE(stable.insert(kbuf, orig));
        ASSERT_TRUE(stable.search(kbuf, &vbuf));
        EXPECT_EQ(strncmp(vbuf(), orig(), orig.size()), 0);
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
