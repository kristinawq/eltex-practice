#ifndef BROKER_H
#define BROKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 12345
#define MAX_PAYLOAD 256
#define MAX_TOPIC 64

typedef struct {
    long mtype;
    char mtext[MAX_PAYLOAD];
} Message;

extern int g_msqid; 

int get_msg_queue(int flags);
int send_message(int msqid, long type, const char *text);

void run_broker();
void run_publisher(pid_t pid, const char *topic);
void run_subscriber(pid_t pid, int topic_count, char **topics);

#endif