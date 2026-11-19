#include "broker.h"

pid_t g_pid;
int g_topic_count;
char **g_topics;

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Подписчик %d] Получен сигнал завершения. Отписываемся...\n", g_pid);
    
    for (int i = 0; i < g_topic_count; i++) {
        char text[MAX_PAYLOAD];
        snprintf(text, sizeof(text), "unsubscribe,%d,%s", g_pid, g_topics[i]);
        send_message(g_msqid, 1, text);
    }
    exit(0);
}

void run_subscriber(pid_t pid, int topic_count, char **topics) {
    g_msqid = get_msg_queue(0666 | IPC_CREAT);
    g_pid = pid;
    g_topic_count = topic_count;
    g_topics = topics;

    if (g_msqid < 0) {
        perror("msgget");
        return;
    }

    for (int i = 0; i < topic_count; i++) {
        char text[MAX_PAYLOAD];
        snprintf(text, sizeof(text), "subscribe,%d,%s", pid, topics[i]);
        send_message(g_msqid, 1, text);
        printf("[Подписчик %d] Подписался на тему: %s\n", pid, topics[i]);
    }

    signal(SIGINT, handle_sigint);

    printf("[Подписчик %d] Ожидает сообщения...\n", pid);

    Message msg;
    while (1) {
        if (msgrcv(g_msqid, &msg, sizeof(msg.mtext), pid, 0) > 0) {
            printf("[Подписчик %d] Получено сообщение: %s\n", pid, msg.mtext);
        }
    }
}