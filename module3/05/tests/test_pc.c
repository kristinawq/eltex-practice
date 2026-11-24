#include <errno.h>
#include <string.h>

#include "producer_consumer.h"
#include "unity.h"

#define NAME "/pc_unity_test"
#define SHM_SIZE 128

static Pc producer, consumer;

void setUp(void)
{
    pc_remove(NAME);
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_create(&producer, NAME, SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_open(&consumer, NAME));
}

void tearDown(void)
{
    pc_close(&consumer, 0);
    pc_close(&producer, 1);
    pc_remove(NAME);
}

static PcBlock *block_at(const Pc *pc, int offset)
{
    return (PcBlock *)(pc->mem + offset);
}

void test_consumer_sees_same_memory_size(void)
{
    TEST_ASSERT_EQUAL_UINT(SHM_SIZE, consumer.size);
}

void test_put_then_take_returns_same_values(void)
{
    int in[] = {5, -3, 9}, out[PC_MAX_VALUES], count = 0;

    TEST_ASSERT_EQUAL_INT(PC_OK, pc_put(&producer, in, 3));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_take(&consumer, out, PC_MAX_VALUES, &count));
    TEST_ASSERT_EQUAL_INT(3, count);
    TEST_ASSERT_EQUAL_INT_ARRAY(in, out, 3);
}

void test_take_from_empty_list_returns_empty(void)
{
    int out[PC_MAX_VALUES], count;

    TEST_ASSERT_EQUAL_INT(PC_EMPTY, pc_take(&consumer, out, PC_MAX_VALUES, &count));
}

void test_set_is_taken_only_once(void)
{
    int in[] = {1, 2}, out[PC_MAX_VALUES], count;

    pc_put(&producer, in, 2);
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_take(&consumer, out, PC_MAX_VALUES, &count));
    TEST_ASSERT_EQUAL_INT(PC_EMPTY, pc_take(&producer, out, PC_MAX_VALUES, &count));
}

void test_taken_set_is_marked_with_zero_count_and_stays_in_list(void)
{
    int in[] = {1, 2}, out[PC_MAX_VALUES], count;
    PcHeader *h = (PcHeader *)producer.mem;

    pc_put(&producer, in, 2);
    pc_take(&consumer, out, PC_MAX_VALUES, &count);

    TEST_ASSERT_NOT_EQUAL(0, h->first);
    TEST_ASSERT_EQUAL_INT(0, block_at(&producer, h->first)->count);
}

void test_blocks_are_linked_and_last_next_is_zero(void)
{
    int in[] = {1, 2, 3};
    PcHeader *h = (PcHeader *)producer.mem;
    PcBlock *first, *second;

    pc_put(&producer, in, 3);
    pc_put(&producer, in, 2);

    first = block_at(&producer, h->first);
    TEST_ASSERT_EQUAL_INT(h->last, first->next);
    second = block_at(&producer, first->next);
    TEST_ASSERT_EQUAL_INT(0, second->next);
    TEST_ASSERT_EQUAL_INT(2, second->count);
}

void test_sets_are_taken_in_order(void)
{
    int a[] = {1}, b[] = {2, 2}, out[PC_MAX_VALUES], count;

    pc_put(&producer, a, 1);
    pc_put(&producer, b, 2);
    pc_take(&consumer, out, PC_MAX_VALUES, &count);
    TEST_ASSERT_EQUAL_INT(1, count);
    pc_take(&consumer, out, PC_MAX_VALUES, &count);
    TEST_ASSERT_EQUAL_INT(2, count);
}

void test_put_returns_full_when_memory_exhausted(void)
{
    int in[PC_MAX_VALUES] = {0}, puts = 0;

    while (pc_put(&producer, in, PC_MAX_VALUES) == PC_OK)
        puts++;

    TEST_ASSERT_EQUAL_INT(1, puts);
    TEST_ASSERT_EQUAL_INT(PC_FULL, pc_put(&producer, in, PC_MAX_VALUES));
}

void test_small_set_still_fits_after_big_one_does_not(void)
{
    int in[PC_MAX_VALUES] = {0};

    pc_put(&producer, in, PC_MAX_VALUES);
    TEST_ASSERT_EQUAL_INT(PC_FULL, pc_put(&producer, in, PC_MAX_VALUES));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_put(&producer, in, 4));
}

void test_unprocessed_counter(void)
{
    int in[] = {1, 2}, out[PC_MAX_VALUES], count;

    TEST_ASSERT_EQUAL_INT(0, pc_unprocessed(&producer));
    pc_put(&producer, in, 2);
    pc_put(&producer, in, 2);
    TEST_ASSERT_EQUAL_INT(2, pc_unprocessed(&producer));
    pc_take(&consumer, out, PC_MAX_VALUES, &count);
    TEST_ASSERT_EQUAL_INT(1, pc_unprocessed(&producer));
}

void test_finished_is_reported_only_after_all_sets_taken(void)
{
    int in[] = {7}, out[PC_MAX_VALUES], count;

    pc_put(&producer, in, 1);
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_finish(&producer));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_take(&consumer, out, PC_MAX_VALUES, &count));
    TEST_ASSERT_EQUAL_INT(PC_FINISHED, pc_take(&consumer, out, PC_MAX_VALUES, &count));
}

void test_two_consumers_never_get_the_same_set(void)
{
    Pc second;
    int in[1], out[PC_MAX_VALUES], count, seen[8] = {0};

    TEST_ASSERT_EQUAL_INT(PC_OK, pc_open(&second, NAME));
    for (int i = 0; i < 8; i++) {
        in[0] = i;
        TEST_ASSERT_EQUAL_INT(PC_OK, pc_put(&producer, in, 1));
    }
    for (int i = 0; i < 8; i++) {
        Pc *taker = (i % 2) ? &second : &consumer;
        TEST_ASSERT_EQUAL_INT(PC_OK, pc_take(taker, out, PC_MAX_VALUES, &count));
        seen[out[0]]++;
    }
    for (int i = 0; i < 8; i++)
        TEST_ASSERT_EQUAL_INT(1, seen[i]);
    pc_close(&second, 0);
}

void test_take_with_small_buffer_fails_and_keeps_set(void)
{
    int in[] = {1, 2, 3}, out[PC_MAX_VALUES], count;

    pc_put(&producer, in, 3);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_take(&consumer, out, 2, &count));
    TEST_ASSERT_EQUAL_INT(EMSGSIZE, errno);
    TEST_ASSERT_EQUAL_INT(1, pc_unprocessed(&producer));
}

void test_put_rejects_bad_arguments(void)
{
    int in[PC_MAX_VALUES + 1] = {0};

    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_put(&producer, in, 0));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_put(&producer, in, PC_MAX_VALUES + 1));
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_put(&producer, NULL, 1));
    TEST_ASSERT_EQUAL_INT(0, pc_unprocessed(&producer));
}

void test_names_get_leading_slash_and_suffixes(void)
{
    TEST_ASSERT_EQUAL_STRING("/pc_unity_test_shm", producer.shm_name);
    TEST_ASSERT_EQUAL_STRING("/pc_unity_test_sem", producer.sem_name);
}

void test_invalid_names_rejected(void)
{
    Pc other;
    char too_long[PC_NAME_SIZE + 10];

    memset(too_long, 'a', sizeof too_long - 1);
    too_long[sizeof too_long - 1] = '\0';
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_create(&other, NULL, SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_create(&other, "", SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_open(&other, too_long));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

void test_create_twice_fails_with_eexist(void)
{
    Pc other;

    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_create(&other, NAME, SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(EEXIST, errno);
}

void test_create_rejects_bad_size(void)
{
    Pc other;

    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_create(&other, NAME, PC_MIN_SHM_SIZE - 1));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_create(&other, NAME, PC_MAX_SHM_SIZE + 1));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

void test_objects_are_removed_after_destroy(void)
{
    Pc other;

    pc_close(&consumer, 0);
    pc_close(&producer, 1);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_open(&other, NAME));

    TEST_ASSERT_EQUAL_INT(PC_OK, pc_create(&producer, NAME, SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_open(&consumer, NAME));
}

void test_open_fails_when_nothing_created(void)
{
    Pc other;

    pc_close(&consumer, 0);
    pc_close(&producer, 1);
    pc_remove(NAME);
    TEST_ASSERT_EQUAL_INT(PC_ERROR, pc_open(&other, NAME));
    TEST_ASSERT_EQUAL_INT(ENOENT, errno);

    TEST_ASSERT_EQUAL_INT(PC_OK, pc_create(&producer, NAME, SHM_SIZE));
    TEST_ASSERT_EQUAL_INT(PC_OK, pc_open(&consumer, NAME));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_consumer_sees_same_memory_size);
    RUN_TEST(test_put_then_take_returns_same_values);
    RUN_TEST(test_take_from_empty_list_returns_empty);
    RUN_TEST(test_set_is_taken_only_once);
    RUN_TEST(test_taken_set_is_marked_with_zero_count_and_stays_in_list);
    RUN_TEST(test_blocks_are_linked_and_last_next_is_zero);
    RUN_TEST(test_sets_are_taken_in_order);
    RUN_TEST(test_put_returns_full_when_memory_exhausted);
    RUN_TEST(test_small_set_still_fits_after_big_one_does_not);
    RUN_TEST(test_unprocessed_counter);
    RUN_TEST(test_finished_is_reported_only_after_all_sets_taken);
    RUN_TEST(test_two_consumers_never_get_the_same_set);
    RUN_TEST(test_take_with_small_buffer_fails_and_keeps_set);
    RUN_TEST(test_put_rejects_bad_arguments);
    RUN_TEST(test_names_get_leading_slash_and_suffixes);
    RUN_TEST(test_invalid_names_rejected);
    RUN_TEST(test_create_twice_fails_with_eexist);
    RUN_TEST(test_create_rejects_bad_size);
    RUN_TEST(test_objects_are_removed_after_destroy);
    RUN_TEST(test_open_fails_when_nothing_created);
    return UNITY_END();
}
