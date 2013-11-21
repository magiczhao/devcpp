#include <gtest/gtest.h>
#include "segment.h"

TEST(SegmentTest, extend)
{
    char buffer[1024 * 1024];
    devcpp::bigmap::Buffer buf(buffer, sizeof(buffer));
    devcpp::bigmap::Segment segment(buf);
    ASSERT_TRUE(segment.init());
    EXPECT_EQ(segment.is_init(1024), true);
    EXPECT_EQ(segment.is_init(4096), false);

    segment.init_slot(4096);
    EXPECT_EQ(segment.is_init(4096), false);
    segment.extend();
    segment.init_slot(4096);
    EXPECT_EQ(segment.is_init(4096), true);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
