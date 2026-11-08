#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "contacts.h"

static ContactList list;

void setUp(void) {
    init_list(&list);
}

void tearDown(void) {
    free_list(&list);
}

Contact make_contact(const char *first, const char *last) {
    Contact c = {0};
    strncpy(c.first_name, first, sizeof(c.first_name) - 1);
    strncpy(c.last_name, last, sizeof(c.last_name) - 1);
    return c;
}

void test_add_and_sort(void) {
    add_contact(&list, make_contact("Иван", "Иванов"));
    add_contact(&list, make_contact("Петр", "Петров"));
    add_contact(&list, make_contact("Анна", "Антонова")); 

    TEST_ASSERT_EQUAL_INT(3, list.count);
    TEST_ASSERT_EQUAL_STRING("Антонова", list.head->data.last_name);
    TEST_ASSERT_EQUAL_STRING("Иванов", list.head->next->data.last_name);
    TEST_ASSERT_EQUAL_STRING("Петров", list.head->next->next->data.last_name);
}

void test_double_linked_connections(void) {
    add_contact(&list, make_contact("A", "A"));
    add_contact(&list, make_contact("B", "B"));

    Node *second = list.head->next;
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL_PTR(list.head, second->prev);
}

void test_delete_head(void) {
    add_contact(&list, make_contact("A", "A"));
    add_contact(&list, make_contact("B", "B"));
    
    delete_contact(&list, 0);
    
    TEST_ASSERT_EQUAL_INT(1, list.count);
    TEST_ASSERT_EQUAL_STRING("B", list.head->data.last_name);
    TEST_ASSERT_NULL(list.head->prev);
}

void test_delete_middle(void) {
    add_contact(&list, make_contact("A", "A"));
    add_contact(&list, make_contact("B", "B"));
    add_contact(&list, make_contact("C", "C"));
    
    delete_contact(&list, 1);
    
    TEST_ASSERT_EQUAL_INT(2, list.count);
    TEST_ASSERT_EQUAL_STRING("A", list.head->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", list.head->next->data.last_name);
    TEST_ASSERT_EQUAL_PTR(list.head, list.head->next->prev);
}

void test_delete_invalid(void) {
    add_contact(&list, make_contact("A", "A"));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(&list, -1));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(&list, 5));
    TEST_ASSERT_EQUAL_INT(1, list.count);
}

void test_edit_contact(void) {
    add_contact(&list, make_contact("A", "A"));
    
    Contact updated = make_contact("NewName", "NewLastName");
    edit_contact(&list, 0, updated);

    TEST_ASSERT_EQUAL_STRING("NewName", list.head->data.first_name);
    TEST_ASSERT_EQUAL_STRING("NewLastName", list.head->data.last_name);
}

void test_empty_list(void) {
    TEST_ASSERT_NULL(list.head);
    TEST_ASSERT_EQUAL_INT(0, list.count);
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(&list, 0));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_and_sort);
    RUN_TEST(test_double_linked_connections);
    RUN_TEST(test_delete_head);
    RUN_TEST(test_delete_middle);
    RUN_TEST(test_delete_invalid);
    RUN_TEST(test_edit_contact);
    RUN_TEST(test_empty_list);
    return UNITY_END();
}
