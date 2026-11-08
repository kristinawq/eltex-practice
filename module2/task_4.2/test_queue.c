#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unity.h"
#include "queue.h"

static PriorityQueue q;

void setUp(void) { init_queue(&q); }
void tearDown(void) { free_queue(&q); }

void test_push_and_order(void) {
    push(&q, 10, "Msg 10");
    push(&q, 5, "Msg 5");
    push(&q, 20, "Msg 20");

    TEST_ASSERT_EQUAL_INT(5, q.head->priority);
    TEST_ASSERT_EQUAL_INT(10, q.head->next->priority);
    TEST_ASSERT_EQUAL_INT(20, q.head->next->next->priority);
}

void test_pop_first(void) {
    push(&q, 10, "A");
    push(&q, 5, "B");
    
    Node *n = pop_first(&q);
    TEST_ASSERT_EQUAL_INT(5, n->priority);
    TEST_ASSERT_EQUAL_STRING("B", n->message);
    free(n);
    
    TEST_ASSERT_EQUAL_INT(1, q.count);
    TEST_ASSERT_EQUAL_INT(10, q.head->priority);
}

void test_pop_by_priority(void) {
    push(&q, 10, "A");
    push(&q, 20, "B");
    push(&q, 30, "C");

    Node *n = pop_by_priority(&q, 20);
    TEST_ASSERT_EQUAL_INT(20, n->priority);
    TEST_ASSERT_EQUAL_STRING("B", n->message);
    free(n);

    TEST_ASSERT_NULL(pop_by_priority(&q, 20));
}

void test_pop_at_least(void) {
    push(&q, 10, "A");
    push(&q, 50, "B");
    
    Node *n = pop_at_least(&q, 30);
    TEST_ASSERT_EQUAL_INT(10, n->priority);
    free(n);
    
    TEST_ASSERT_NULL(pop_at_least(&q, 30));
}

void test_empty_queue(void) {
    TEST_ASSERT_NULL(pop_first(&q));
    TEST_ASSERT_NULL(pop_by_priority(&q, 10));
    TEST_ASSERT_NULL(pop_at_least(&q, 10));
    TEST_ASSERT_EQUAL_INT(0, q.count);
}

void test_invalid_priority(void) {
    TEST_ASSERT_EQUAL_INT(-1, push(&q, -5, "Bad"));
    TEST_ASSERT_EQUAL_INT(-1, push(&q, 300, "Bad"));
    TEST_ASSERT_EQUAL_INT(0, q.count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_push_and_order);
    RUN_TEST(test_pop_first);
    RUN_TEST(test_pop_by_priority);
    RUN_TEST(test_pop_at_least);
    RUN_TEST(test_empty_queue);
    RUN_TEST(test_invalid_priority);
    return UNITY_END();
}
