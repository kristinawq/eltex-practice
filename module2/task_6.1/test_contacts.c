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

void test_init_empty(void) {
    TEST_ASSERT_NULL(list.head);
    TEST_ASSERT_EQUAL_INT(0, list.count);
}

void test_add_contact_normal(void) {
    Contact c = make_contact("Иван", "Иванов");
    TEST_ASSERT_EQUAL_INT(0, add_contact(&list, c));
    TEST_ASSERT_EQUAL_INT(1, list.count);
    TEST_ASSERT_EQUAL_STRING("Иван", list.head->data.first_name);
    TEST_ASSERT_EQUAL_STRING("Иванов", list.head->data.last_name);
}

void test_add_contact_sorted(void) {
    add_contact(&list, make_contact("Пётр", "Сидоров"));
    add_contact(&list, make_contact("Иван", "Иванов"));
    add_contact(&list, make_contact("Анна", "Алексеева"));
    
    TEST_ASSERT_EQUAL_INT(3, list.count);
    
    TEST_ASSERT_EQUAL_STRING("Алексеева", list.head->data.last_name);
    TEST_ASSERT_EQUAL_STRING("Иванов", list.head->next->data.last_name);
    TEST_ASSERT_EQUAL_STRING("Сидоров", list.head->next->next->data.last_name);
}

void test_delete_contact_valid(void) {
    add_contact(&list, make_contact("Анна", "Алексеева"));
    add_contact(&list, make_contact("Иван", "Иванов"));
    add_contact(&list, make_contact("Пётр", "Сидоров"));
    
    TEST_ASSERT_EQUAL_INT(0, delete_contact(&list, 1));
    TEST_ASSERT_EQUAL_INT(2, list.count);
    
    TEST_ASSERT_EQUAL_STRING("Алексеева", list.head->data.last_name);
    TEST_ASSERT_EQUAL_STRING("Сидоров", list.head->next->data.last_name);
}

void test_delete_contact_invalid(void) {
    add_contact(&list, make_contact("Иван", "Иванов"));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(&list, -1));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(&list, 5));
    TEST_ASSERT_EQUAL_INT(1, list.count);
}

void test_edit_contact_valid(void) {
    add_contact(&list, make_contact("Иван", "Иванов"));
    Contact updated = make_contact("Пётр", "Петров");
    
    TEST_ASSERT_EQUAL_INT(0, edit_contact(&list, 0, updated));
    TEST_ASSERT_EQUAL_INT(1, list.count);
    TEST_ASSERT_EQUAL_STRING("Пётр", list.head->data.first_name);
    TEST_ASSERT_EQUAL_STRING("Петров", list.head->data.last_name);
}

void test_edit_contact_invalid(void) {
    TEST_ASSERT_EQUAL_INT(-1, edit_contact(&list, 0, make_contact("Анна", "Алексеева")));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_empty);
    RUN_TEST(test_add_contact_normal);
    RUN_TEST(test_add_contact_sorted);
    RUN_TEST(test_delete_contact_valid);
    RUN_TEST(test_delete_contact_invalid);
    RUN_TEST(test_edit_contact_valid);
    RUN_TEST(test_edit_contact_invalid);
    return UNITY_END();
}
