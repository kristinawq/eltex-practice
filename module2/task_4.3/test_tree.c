#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "tree.h"

static AVLNode *root;

void setUp(void) {
    root = NULL;
}

void tearDown(void) {
    free_tree(root);
}

Contact make_contact(const char *first, const char *last) {
    Contact c = {0};
    strncpy(c.first_name, first, sizeof(c.first_name) - 1);
    strncpy(c.last_name, last, sizeof(c.last_name) - 1);
    return c;
}

void test_insert_and_balance_ll(void) {
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("C", "C"));
    
    TEST_ASSERT_EQUAL_STRING("B", root->data.last_name);
    TEST_ASSERT_EQUAL_INT(2, root->height);
    TEST_ASSERT_EQUAL_STRING("A", root->left->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", root->right->data.last_name);
}

void test_insert_and_balance_rr(void) {
    root = insert_node(root, make_contact("C", "C"));
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("A", "A"));
    
    TEST_ASSERT_EQUAL_STRING("B", root->data.last_name);
    TEST_ASSERT_EQUAL_STRING("A", root->left->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", root->right->data.last_name);
}

void test_insert_and_balance_lr(void) {
    root = insert_node(root, make_contact("C", "C"));
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("B", "B"));
    
    TEST_ASSERT_EQUAL_STRING("B", root->data.last_name);
    TEST_ASSERT_EQUAL_STRING("A", root->left->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", root->right->data.last_name);
}

void test_insert_and_balance_rl(void) {
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("C", "C"));
    root = insert_node(root, make_contact("B", "B"));
    
    TEST_ASSERT_EQUAL_STRING("B", root->data.last_name);
    TEST_ASSERT_EQUAL_STRING("A", root->left->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", root->right->data.last_name);
}

void test_insert_duplicate(void) {
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("A", "A"));
    TEST_ASSERT_EQUAL_INT(1, root->height);
}

void test_find_existing(void) {
    root = insert_node(root, make_contact("Иван", "Иванов"));
    root = insert_node(root, make_contact("Петр", "Петров"));
    
    int steps = 0;
    AVLNode *found = find_node_debug(root, make_contact("Петр", "Петров"), &steps);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STRING("Петров", found->data.last_name);
    TEST_ASSERT_GREATER_THAN(0, steps);
}

void test_find_not_existing(void) {
    root = insert_node(root, make_contact("Иван", "Иванов"));
    
    int steps = 0;
    AVLNode *found = find_node_debug(root, make_contact("Вася", "Пупкин"), &steps);
    TEST_ASSERT_NULL(found);
}

void test_find_in_empty_tree(void) {
    int steps = 0;
    AVLNode *found = find_node_debug(root, make_contact("A", "A"), &steps);
    TEST_ASSERT_NULL(found);
}

void test_delete_leaf(void) {
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("C", "C"));
    
    root = delete_node(root, make_contact("A", "A"));
    TEST_ASSERT_NULL(root->left);
    TEST_ASSERT_EQUAL_INT(2, root->height);
}

void test_delete_node_with_one_child(void) {
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("A", "A"));
    
    root = delete_node(root, make_contact("B", "B"));
    TEST_ASSERT_EQUAL_STRING("A", root->data.last_name);
    TEST_ASSERT_EQUAL_INT(1, root->height);
}

void test_delete_node_with_two_children(void) {
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("C", "C"));
    
    root = delete_node(root, make_contact("B", "B"));
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL_INT(2, root->height);
}

void test_delete_root_and_balance(void) {
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("C", "C"));
    
    root = delete_node(root, make_contact("B", "B"));
    TEST_ASSERT_NOT_NULL(root);
}

void test_delete_non_existent(void) {
    root = insert_node(root, make_contact("A", "A"));
    root = delete_node(root, make_contact("X", "X"));
    TEST_ASSERT_EQUAL_INT(1, root->height);
}

void test_delete_from_empty(void) {
    root = delete_node(root, make_contact("A", "A"));
    TEST_ASSERT_NULL(root);
}

void test_traversals(void) {
    root = insert_node(root, make_contact("B", "B"));
    root = insert_node(root, make_contact("A", "A"));
    root = insert_node(root, make_contact("C", "C"));
    
    TEST_ASSERT_EQUAL_STRING("B", root->data.last_name);
    TEST_ASSERT_EQUAL_STRING("A", root->left->data.last_name);
    TEST_ASSERT_EQUAL_STRING("C", root->right->data.last_name);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_insert_and_balance_ll);
    RUN_TEST(test_insert_and_balance_rr);
    RUN_TEST(test_insert_and_balance_lr);
    RUN_TEST(test_insert_and_balance_rl);
    RUN_TEST(test_insert_duplicate);
    
    RUN_TEST(test_find_existing);
    RUN_TEST(test_find_not_existing);
    RUN_TEST(test_find_in_empty_tree);
    
    RUN_TEST(test_delete_leaf);
    RUN_TEST(test_delete_node_with_one_child);
    RUN_TEST(test_delete_node_with_two_children);
    RUN_TEST(test_delete_root_and_balance);
    RUN_TEST(test_delete_non_existent);
    RUN_TEST(test_delete_from_empty);
    
    RUN_TEST(test_traversals);
    
    return UNITY_END();
}
