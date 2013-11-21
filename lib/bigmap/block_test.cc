#include <gtest/gtest.h>
#include "block.h"

int full_compare(const char* str1, int size1, const char* str2, int size2)
{
    if(size1 == size2 && strncmp(str1, str2, size1) == 0){
        return 0;
    }
    return 1;
}

TEST(BlockTest, InsertToNull)
{
    char buffer[4096];
    devcpp::bigmap::BlockManager manager(0);
    ASSERT_TRUE(manager.init(string("./tmp_test.dat")));
    devcpp::bigmap::Block* block = manager.get(1);
    ASSERT_TRUE(block);
    ASSERT_TRUE(block->init(1));
    EXPECT_EQ(block->insert("abc", 3), 0);
    EXPECT_EQ(block->insert("abcde", 5), 1);

    EXPECT_EQ(block->find("abc", 3, full_compare), 0);
    EXPECT_EQ(block->size(), 2);
    delete block;
}

TEST(Iterate, Iterator)
{
    char buffer[4096];
    devcpp::bigmap::BlockManager manager(0);
    ASSERT_TRUE(manager.init(string("./tmp_iter.dat")));
    devcpp::bigmap::Block* block = manager.get(1);
    ASSERT_TRUE(block);
    ASSERT_TRUE(block->init(1));
    EXPECT_EQ(block->insert("0", 1), 0);
    EXPECT_EQ(block->insert("1", 1), 1);

    devcpp::bigmap::Block::Iterator it = block->begin();
    for(; it != block->end(); ++it)
    {
    }
}

TEST(BlockTest, InsertToNotNull)
{}
int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
