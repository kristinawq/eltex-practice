#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "contacts.h"

static Contact contacts[MAX_CONTACTS];
static int count;

void setUp(void) {
    count = 0;
    memset(contacts, 0, sizeof(contacts));
}

void tearDown(void) {}

Contact make_contact(const char *first, const char *last) {
    Contact c = {0};
    strncpy(c.first_name, first, sizeof(c.first_name) - 1);
    strncpy(c.last_name, last, sizeof(c.last_name) - 1);
    return c;
}

void test_add_contact_normal(void) {
    Contact c = make_contact("Ivan", "Ivanov");
    TEST_ASSERT_EQUAL_INT(0, add_contact(contacts, &count, c));
    TEST_ASSERT_EQUAL_INT(1, count);
    TEST_ASSERT_EQUAL_STRING("Ivan", contacts[0].first_name);
}

void test_add_contact_overflow(void) {
    Contact c = make_contact("A", "B");
    for (int i = 0; i < MAX_CONTACTS; i++) {
        add_contact(contacts, &count, c);
    }
    TEST_ASSERT_EQUAL_INT(-1, add_contact(contacts, &count, c));
}

void test_delete_contact_valid(void) {
    add_contact(contacts, &count, make_contact("A", "A"));
    add_contact(contacts, &count, make_contact("B", "B"));
    add_contact(contacts, &count, make_contact("C", "C"));
    
    TEST_ASSERT_EQUAL_INT(0, delete_contact(contacts, &count, 1));
    TEST_ASSERT_EQUAL_INT(2, count);
    TEST_ASSERT_EQUAL_STRING("A", contacts[0].first_name);
    TEST_ASSERT_EQUAL_STRING("C", contacts[1].first_name);
}

void test_delete_contact_invalid(void) {
    add_contact(contacts, &count, make_contact("A", "A"));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(contacts, &count, -1));
    TEST_ASSERT_EQUAL_INT(-1, delete_contact(contacts, &count, 5));
}

void test_edit_contact_valid(void) {
    add_contact(contacts, &count, make_contact("Old", "Name"));
    Contact updated = make_contact("New", "Name");
    
    TEST_ASSERT_EQUAL_INT(0, edit_contact(contacts, count, 0, updated));
    TEST_ASSERT_EQUAL_STRING("New", contacts[0].first_name);
}

void test_edit_contact_invalid(void) {
    TEST_ASSERT_EQUAL_INT(-1, edit_contact(contacts, count, 0, make_contact("A", "B")));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_contact_normal);
    RUN_TEST(test_add_contact_overflow);
    RUN_TEST(test_delete_contact_valid);
    RUN_TEST(test_delete_contact_invalid);
    RUN_TEST(test_edit_contact_valid);
    RUN_TEST(test_edit_contact_invalid);
    return UNITY_END();
}
