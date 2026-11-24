#ifndef CHAT_H
#define CHAT_H

#include <mqueue.h>

#define CHAT_MSG_SIZE         256
#define CHAT_BUF_SIZE         (CHAT_MSG_SIZE + 1)
#define CHAT_MAX_MSGS         10
#define CHAT_PRIO_MSG         1
#define CHAT_PRIO_QUIT        10
#define CHAT_SEND_TIMEOUT_SEC 1
#define CHAT_NAME_SIZE        256

typedef struct {
    mqd_t rx;
    mqd_t tx;
    int owner;
    char name_1[CHAT_NAME_SIZE];
    char name_2[CHAT_NAME_SIZE];
} chat_t;

int chat_open(chat_t *chat, const char *name);

int chat_send(const chat_t *chat, const char *text);

int chat_send_quit(const chat_t *chat);

int chat_receive(const chat_t *chat, char *buf, int *quit);

void chat_close(chat_t *chat);

#endif
