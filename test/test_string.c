#include <stdlib.h>
#include <string.h>
#include "dstring.h"
#include <check.h>
char* big = "So there you have it. Like I promised before,it is no exhaustive explanation or formal proof of KMP; it's a walk through my brain, with the parts I found confusing spelled out in extreme detail. If you have any questions or notice something I messed up, please leave a comment; maybe we'll all learn something.";
struct test_match{
    char* str;
    int expected;
};
struct test_match tests[] = {
        {"So", 0},
        {"have it", 13},
        {"we'll", 284},
        {"ormal proo", 81},
        {"exhaustive ex", 54},
        {"mmmm", -1},
        {NULL, 0}
    };
START_TEST(test_kmp)
{
    struct dstring haystack;
    struct dstring needle;
    dstring_assign(&haystack, big, strlen(big));
    int index = 0;
    while(1){
        if(tests[index].str){
            dstring_assign(&needle, tests[index].str, strlen(tests[index].str));
            int pos = kmp_search(&haystack, &needle);
            fail_unless(pos == tests[index].expected, "test case %d failed, pos:%d, string:%s", index, pos, tests[index].str);
        }else{break;}
        index++;
    }
}
END_TEST

Suite* dstring_suite()
{
    Suite* s = suite_create("dstring");
    TCase* dscase = tcase_create("op");
    tcase_add_test(dscase, test_kmp);
    suite_add_tcase(s, dscase);
    return s;
}

int main()
{
    Suite* s = dstring_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
