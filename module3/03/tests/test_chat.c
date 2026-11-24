#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "chat.h"
#include "unity.h"

#define QUEUE "unity_test_chat"

static chat_t owner, guest;

static int queue_exists(const char *name)
{
    mqd_t mq = mq_open(name, O_RDONLY);
    if (mq == (mqd_t)-1)
        return 0;
    mq_close(mq);
    return 1;
}

void setUp(void)
{
    TEST_ASSERT_EQUAL_INT(0, chat_open(&owner, QUEUE));
    TEST_ASSERT_EQUAL_INT(0, chat_open(&guest, QUEUE));
}

void tearDown(void)
{
    chat_close(&guest);
    chat_close(&owner);
}

void test_first_is_owner_second_is_guest(void)
{
    TEST_ASSERT_TRUE(owner.owner);
    TEST_ASSERT_FALSE(guest.owner);
}

void test_names_get_leading_slash_and_suffixes(void)
{
    TEST_ASSERT_EQUAL_STRING("/" QUEUE "_1", owner.name_1);
    TEST_ASSERT_EQUAL_STRING("/" QUEUE "_2", owner.name_2);
}

void test_owner_to_guest(void)
{
    char buf[CHAT_BUF_SIZE];
    int quit = -1;

    TEST_ASSERT_EQUAL_INT(0, chat_send(&owner, "привет"));
    TEST_ASSERT_EQUAL_INT((int)strlen("привет"), chat_receive(&guest, buf, &quit));
    TEST_ASSERT_EQUAL_STRING("привет", buf);
    TEST_ASSERT_EQUAL_INT(0, quit);
}

void test_guest_to_owner(void)
{
    char buf[CHAT_BUF_SIZE];
    int quit = -1;

    TEST_ASSERT_EQUAL_INT(0, chat_send(&guest, "hello"));
    TEST_ASSERT_EQUAL_INT(5, chat_receive(&owner, buf, &quit));
    TEST_ASSERT_EQUAL_STRING("hello", buf);
    TEST_ASSERT_EQUAL_INT(0, quit);
}

void test_directions_are_independent(void)
{
    char buf[CHAT_BUF_SIZE];
    int quit;

    chat_send(&owner, "from owner");
    chat_send(&guest, "from guest");

    chat_receive(&owner, buf, &quit);
    TEST_ASSERT_EQUAL_STRING("from guest", buf);
    chat_receive(&guest, buf, &quit);
    TEST_ASSERT_EQUAL_STRING("from owner", buf);
}

void test_messages_keep_order(void)
{
    char buf[CHAT_BUF_SIZE];
    int quit;

    chat_send(&owner, "one");
    chat_send(&owner, "two");
    chat_receive(&guest, buf, &quit);
    TEST_ASSERT_EQUAL_STRING("one", buf);
    chat_receive(&guest, buf, &quit);
    TEST_ASSERT_EQUAL_STRING("two", buf);
}

void test_quit_has_higher_priority_than_text(void)
{
    char buf[CHAT_BUF_SIZE];
    int quit = 0;

    chat_send(&guest, "a");
    chat_send(&guest, "b");
    chat_send_quit(&guest);

    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, chat_receive(&owner, buf, &quit));
    TEST_ASSERT_EQUAL_INT(1, quit);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, chat_receive(&owner, buf, &quit));
    TEST_ASSERT_EQUAL_INT(0, quit);
}

void test_message_longer_than_limit_is_rejected(void)
{
    char big[CHAT_MSG_SIZE + 10];
    memset(big, 'x', sizeof big - 1);
    big[sizeof big - 1] = '\0';

    TEST_ASSERT_EQUAL_INT(-1, chat_send(&owner, big));
    TEST_ASSERT_EQUAL_INT(EMSGSIZE, errno);
}

void test_max_size_message_is_accepted(void)
{
    char text[CHAT_MSG_SIZE], buf[CHAT_BUF_SIZE];
    int quit;

    memset(text, 'x', sizeof text - 1);
    text[sizeof text - 1] = '\0';

    TEST_ASSERT_EQUAL_INT(0, chat_send(&owner, text));
    TEST_ASSERT_EQUAL_INT(CHAT_MSG_SIZE - 1, chat_receive(&guest, buf, &quit));
    TEST_ASSERT_EQUAL_STRING(text, buf);
}

void test_send_to_full_queue_times_out(void)
{
    for (int i = 0; i < CHAT_MAX_MSGS; i++)
        TEST_ASSERT_EQUAL_INT(0, chat_send(&owner, "x"));

    TEST_ASSERT_EQUAL_INT(-1, chat_send(&owner, "overflow"));
    TEST_ASSERT_EQUAL_INT(ETIMEDOUT, errno);
}

void test_guest_close_keeps_queues(void)
{
    chat_close(&guest);
    TEST_ASSERT_TRUE(queue_exists(owner.name_1));
    TEST_ASSERT_TRUE(queue_exists(owner.name_2));
    TEST_ASSERT_EQUAL_INT(0, chat_open(&guest, QUEUE));
}

void test_owner_close_removes_queues(void)
{
    char n1[CHAT_NAME_SIZE], n2[CHAT_NAME_SIZE];
    strcpy(n1, owner.name_1);
    strcpy(n2, owner.name_2);

    chat_close(&owner);
    TEST_ASSERT_FALSE(queue_exists(n1));
    TEST_ASSERT_FALSE(queue_exists(n2));

    TEST_ASSERT_EQUAL_INT(0, chat_open(&owner, QUEUE));
}

void test_invalid_names_rejected(void)
{
    chat_t c;
    char too_long[CHAT_NAME_SIZE + 10];
    memset(too_long, 'a', sizeof too_long - 1);
    too_long[sizeof too_long - 1] = '\0';

    TEST_ASSERT_EQUAL_INT(-1, chat_open(&c, NULL));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(-1, chat_open(&c, ""));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
    TEST_ASSERT_EQUAL_INT(-1, chat_open(&c, too_long));
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_first_is_owner_second_is_guest);
    RUN_TEST(test_names_get_leading_slash_and_suffixes);
    RUN_TEST(test_owner_to_guest);
    RUN_TEST(test_guest_to_owner);
    RUN_TEST(test_directions_are_independent);
    RUN_TEST(test_messages_keep_order);
    RUN_TEST(test_quit_has_higher_priority_than_text);
    RUN_TEST(test_message_longer_than_limit_is_rejected);
    RUN_TEST(test_max_size_message_is_accepted);
    RUN_TEST(test_send_to_full_queue_times_out);
    RUN_TEST(test_guest_close_keeps_queues);
    RUN_TEST(test_owner_close_removes_queues);
    RUN_TEST(test_invalid_names_rejected);
    return UNITY_END();
}