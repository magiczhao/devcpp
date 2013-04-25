#include "dlock.h"
#include <check.h>
START_TEST(test_lock)
{
    dlock_t lock;
    int ret = dlock_init(&lock);
    fail_unless(ret == 0, "init lock failed");
    dlock_lock(&lock);
    ret = dlock_trylock(&lock);
    fail_unless(ret != 0, "try lock failed");
    dlock_unlock(&lock);
    ret = dlock_trylock(&lock);
    fail_unless(ret == 0, "try lock unlocked failed");
    dlock_unlock(&lock);
}
END_TEST
Suite* lock_suite()
{
    Suite* s = suite_create("dlock");
    TCase* lockcase = tcase_create("op");
    tcase_add_test(lockcase, test_lock);
    suite_add_tcase(s, lockcase);
    return s;
}

int main()
{
    Suite* s = lock_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
