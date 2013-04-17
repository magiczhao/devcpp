#include "check.h"
#include "bloom_filter.h"
START_TEST(test_bloom_filter)
{}
END_TEST

Suite* bloomfilter_suite()
{
    Suite* s = suite_create("bloomfilter");
    TCase* bfcase = tcase_create("bftest");
    tcase_add_test(bfcase, test_bloom_filter);
    suite_add_tcase(s, bfcase);
    return s;
}

int main()
{
    Suite* s = bloomfilter_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
