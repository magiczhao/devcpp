#include <stdlib.h>
#include <check.h>
#include "fixed_alloc.h"

struct fixed_dmem_pool pool;
void setup()
{
    int ret = fixed_pool_init(&pool, 1000000, 128);
    //fail_unless(ret == 0, "init failed");
}

START_TEST(test_malloc)
{
    void* ptr[10000];
    for(int i = 0; i < 1000; ++i){
        for(int j = 0; j < 10000; ++j){
            ptr[j] = d_fp_malloc(&pool, 100);
        }
        for(int j = 9999; j >= 0; --j){
            d_fp_free(&pool, ptr[j]);
        }
    }
}
END_TEST

START_TEST(test_fini)
{
    fixed_pool_fini(&pool);
}
END_TEST

Suite* fixed_alloc_suite()
{
    Suite* s = suite_create("alloc");
    TCase* simple = tcase_create("simple");
    tcase_add_test(simple, test_malloc);
    tcase_add_test(simple, test_fini);
    suite_add_tcase(s, simple);
    return s;
}
int main()
{
    setup();
    Suite* s = fixed_alloc_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
