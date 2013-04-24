#include <stdlib.h>
#include <check.h>
#include <dhash_table.h>

int comp(char* p1, int l1, char* p2, int l2)
{
    if(p1 == p2 && l1 == l2) return 0;
    return 1;
}

START_TEST(test_ht_init)
{
    struct dhash_table ht;
    int ret;
    ret = dhash_table_init(&ht, 1024 * 1024, OriginHash, comp);
    fail_unless(ret == 0, "init hash table failed");
    dhash_table_fini(&ht);
}
END_TEST

START_TEST(test_ht_insert)
{
    struct dhash_table ht;
    int ret;
    void* value;
    char* buffer = "this is test";
    dhash_table_init(&ht, 1024 * 1024, OriginHash, comp);
    for(int i = 0; i < 1000000; i+=3){
        ret = dhash_table_add(&ht, (char*)i, sizeof(i), buffer);
        fail_unless(ret == 0, "insert hash table failed i=%d, %s", i, strerror(errno));
    }
    for(int i = 0; i < 1000000; i+= 3){
        ret = dhash_table_find(&ht, (char*)i, sizeof(i), &value);
        fail_unless(ret == true, "find hash table failed i=%d", i);
    }
    for(int i = 1; i < 1000000; i+= 3){
        ret = dhash_table_find(&ht, (char*)i, sizeof(i), &value);
        fail_unless(ret == false, "find hash table failed i=%d", i);
    }
}
END_TEST

START_TEST(test_ht_remove)
{
}
END_TEST

Suite* ht_suite()
{
    Suite* s = suite_create("hash_table");
    TCase* htcase = tcase_create("op");
    tcase_add_test(htcase, test_ht_init);
    tcase_add_test(htcase, test_ht_insert);
    tcase_add_test(htcase, test_ht_remove);
    suite_add_tcase(s, htcase);
    return s;
}
int main()
{
    Suite* s = ht_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed;
}
