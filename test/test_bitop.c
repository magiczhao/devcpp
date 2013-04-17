#include <stdlib.h>
#include <check.h>
#include "bit.h"
START_TEST(test_set_bit)
{
    char cval = 0;
    set_bit(&cval, 3);
    fail_unless(cval == 8, "set bit failed");
    cval = 0;
    set_bit(&cval, 0);
    fail_unless(cval == 1, "set zero bit failed");

    int ival = 0;
    set_bit(&ival, 10);
    fail_unless(ival == 1024, "set int failed");
}
END_TEST

Suite* bit_suite()
{
    Suite* s = suite_create("bitop");
    TCase* bitcase = tcase_create("op");
    tcase_add_test(bitcase, test_set_bit);
    suite_add_tcase(s, bitcase);
    return s;
}
int main()
{
    Suite* s = bit_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
