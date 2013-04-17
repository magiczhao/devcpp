#include "check.h"
#include "bloom_filter.h"
struct dbloom_filter gbfilter;
void setup(void)
{
    dbloom_filter_init(&gbfilter, 10000, 40);
}

void teardown(void)
{
    dbloom_filter_fini(&gbfilter);
}

START_TEST(test_bloom_init)
{
    struct dbloom_filter filter;
    int ret = dbloom_filter_init(&filter, 100000, 100);
    fail_unless(ret == 0, "init failed");
    dbloom_filter_fini(&filter);
}
END_TEST

START_TEST(test_bloom_filter)
{
    struct dbloom_filter filter;
    dbloom_filter_init(&filter, 10000, 40);
    static char* inarray[] = {
        "this is test",
        "all is here",
        "devlib",
        NULL
    };
    static char* notinarray[] = {
        "not in here",
        "hh",
        NULL
    };
    for(int i = 0; inarray[i] != NULL; ++i){
        dbloom_filter_add(&filter, inarray[i], strlen(inarray[i]));
    }
    for(int i = 0; notinarray[i] != NULL; ++i){
        int ret = dbloom_filter_find(&filter, notinarray[i], strlen(notinarray[i]));
        fail_unless(ret == 0, "find failed, index:%d, ret: %d!", i, ret);
    }

    for(int i = 0; inarray[i] != NULL; ++i){
        int ret = dbloom_filter_find(&filter, inarray[i], strlen(inarray[i]));
        fail_unless(ret == 1, "find failed, index:%d, ret:%d!", i, ret);
    }
    dbloom_filter_fini(&filter);
}
END_TEST

Suite* bloomfilter_suite()
{
    Suite* s = suite_create("bloomfilter");
    TCase* bfcase = tcase_create("bftest");
    tcase_add_checked_fixture(bfcase, setup, teardown);
    tcase_add_test(bfcase, test_bloom_init);
    tcase_add_test(bfcase, test_bloom_filter);
    suite_add_tcase(s, bfcase);
    return s;
}

int main()
{
    setup();
    Suite* s = bloomfilter_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
