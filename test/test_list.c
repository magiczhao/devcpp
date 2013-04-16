#include <stdlib.h>
#include <check.h>
#include "dlist.h"
struct dlist_node ghead;
struct list_int
{
    struct dlist_node node;
    int value;
};

void setup(void)
{
    INIT_DLIST_HEAD(&ghead);
}

START_TEST(test_init_list)
{
    struct dlist_node head;
    INIT_DLIST_HEAD(&head);
    fail_unless(DLIST_EMPTY(&head), "list init failed!");
}
END_TEST

START_TEST(test_add_list)
{
    for(int i = 0; i < 10; ++i){
        struct list_int* node = (struct list_int*)malloc(sizeof(struct list_int));
        node->value = i;
        dlist_add(&ghead, &node->node);
    }
    struct dlist_node* n;
    int value = 9;
    dlist_for_each(n, &ghead){
        struct list_int* node = container_of(n, struct list_int, node);
        fail_unless(value == node->value, "value failed, expected:%d, value:%d", value, node->value);
        value -= 1;
    }
}
END_TEST

START_TEST(test_del_list)
{
    while(!DLIST_EMPTY(&ghead)){
        struct dlist_node* lnode = dlist_del(ghead.next);
        struct list_int* node = container_of(lnode, struct list_int, node);
        free(node);
    }
    fail_unless(DLIST_EMPTY(&ghead), "list del not empty");
}
END_TEST

Suite* dlist_suite()
{
    Suite* s = suite_create("dlist");
    TCase* simple = tcase_create("simple");
    tcase_add_test(simple, test_init_list);
    tcase_add_test(simple, test_add_list);
    tcase_add_test(simple, test_del_list);
    suite_add_tcase(s, simple);
    return s;
}
int main()
{
    setup();
    Suite* s = dlist_suite();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return number_failed;
}
