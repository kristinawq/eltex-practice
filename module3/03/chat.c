#include "chat.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OPEN_RETRIES   50
#define OPEN_DELAY_US  100000

static const mqd_t INVALID_MQ = (mqd_t)-1;

static int make_names(chat_t *chat, const char *name)
{
    if (name == NULL || name[0] == '\0')
        return -1;

    const char *slash = (name[0] == '/') ? "" : "/";
    int n1 = snprintf(chat->name_1, CHAT_NAME_SIZE, "%s%s_1", slash, name);
    int n2 = snprintf(chat->name_2, CHAT_NAME_SIZE, "%s%s_2", slash, name);
    return (n1 >= CHAT_NAME_SIZE || n2 >= CHAT_NAME_SIZE) ? -1 : 0;
}

static mqd_t open_reader(const char *name)
{
    for (int i = 0; i < OPEN_RETRIES; i++) {
        mqd_t mq = mq_open(name, O_RDONLY);
        if (mq != INVALID_MQ || errno != ENOENT)
            return mq;
        usleep(OPEN_DELAY_US);
    }
    return INVALID_MQ;
}

static int create_queues(chat_t *chat)
{
    struct mq_attr attr = { .mq_maxmsg = CHAT_MAX_MSGS, .mq_msgsize = CHAT_MSG_SIZE };

    chat->rx = mq_open(chat->name_1, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
    if (chat->rx == INVALID_MQ)
        return -1;

    chat->tx = mq_open(chat->name_2, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    if (chat->tx == INVALID_MQ) {
        int saved = errno;
        mq_close(chat->rx);
        mq_unlink(chat->name_1);
        errno = saved;
        return -1;
    }
    chat->owner = 1;
    return 0;
}

static int join_queues(chat_t *chat)
{
    chat->tx = mq_open(chat->name_1, O_WRONLY);
    if (chat->tx == INVALID_MQ)
        return -1;

    chat->rx = open_reader(chat->name_2);
    if (chat->rx == INVALID_MQ) {
        int saved = errno;
        mq_close(chat->tx);
        errno = saved;
        return -1;
    }
    chat->owner = 0;
    return 0;
}

int chat_open(chat_t *chat, const char *name)
{
    if (make_names(chat, name) < 0) {
        errno = EINVAL;
        return -1;
    }
    if (create_queues(chat) == 0)
        return 0;
    return (errno == EEXIST) ? join_queues(chat) : -1;
}

static int send_with_prio(const chat_t *chat, const char *text, unsigned prio)
{
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += CHAT_SEND_TIMEOUT_SEC;

    return mq_timedsend(chat->tx, text, strlen(text) + 1, prio, &deadline);
}

int chat_send(const chat_t *chat, const char *text)
{
    return send_with_prio(chat, text, CHAT_PRIO_MSG);
}

int chat_send_quit(const chat_t *chat)
{
    return send_with_prio(chat, "", CHAT_PRIO_QUIT);
}

int chat_receive(const chat_t *chat, char *buf, int *quit)
{
    unsigned prio;
    ssize_t len = mq_receive(chat->rx, buf, CHAT_BUF_SIZE, &prio);
    if (len < 0)
        return -1;

    buf[len] = '\0';
    *quit = (prio == CHAT_PRIO_QUIT);
    return (int)strlen(buf);
}

void chat_close(chat_t *chat)
{
    mq_close(chat->rx);
    mq_close(chat->tx);
    if (chat->owner) {
        mq_unlink(chat->name_1);
        mq_unlink(chat->name_2);
    }
}
