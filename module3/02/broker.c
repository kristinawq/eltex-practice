#include "broker.h"

int get_msg_queue(int flags) {
    return msgget(MSG_KEY, flags);
}

int send_message(int msqid, long type, const char *text) {
    Message msg;
    msg.mtype = type;
    strncpy(msg.mtext, text, MAX_PAYLOAD - 1);
    msg.mtext[MAX_PAYLOAD - 1] = '\0';
    return msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
}

typedef struct {
    pid_t pid;
    char topics[10][MAX_TOPIC];
    int topic_count;
} SubscriberInfo;

SubscriberInfo subs[50];
int sub_count = 0;
pid_t publishers[50];
int pub_count = 0;

void handle_sigint_broker(int sig) {
    (void)sig;
    printf("\n[Брокер] Завершение работы. Рассылка сигналов...\n");
    for (int i = 0; i < sub_count; i++) kill(subs[i].pid, SIGINT);
    for (int i = 0; i < pub_count; i++) kill(publishers[i], SIGINT);
    msgctl(g_msqid, IPC_RMID, NULL);
    exit(0);
}

void run_broker() {
    g_msqid = msgget(MSG_KEY, 0666 | IPC_CREAT | IPC_EXCL);
    if (g_msqid < 0) {
        perror("Брокер уже запущен или ошибка msgget");
        exit(1);
    }

    signal(SIGINT, handle_sigint_broker);
    printf("[Брокер] Запущен. ID очереди: %d\n", g_msqid);

    Message msg;
    while (1) {
        if (msgrcv(g_msqid, &msg, sizeof(msg.mtext), 1, 0) < 0) {
            perror("msgrcv");
            break;
        }

        char cmd[16], topic[MAX_TOPIC];
        pid_t pid;
        sscanf(msg.mtext, "%[^,],%d,%s", cmd, &pid, topic);

        if (strcmp(cmd, "subscribe") == 0) {
            int found = 0;
            for (int i = 0; i < sub_count; i++) {
                if (subs[i].pid == pid) {
                    strcpy(subs[i].topics[subs[i].topic_count++], topic);
                    found = 1;
                    break;
                }
            }
            if (!found && sub_count < 50) {
                subs[sub_count].pid = pid;
                strcpy(subs[sub_count].topics[0], topic);
                subs[sub_count].topic_count = 1;
                sub_count++;
            }
            printf("[Брокер] Подписчик %d подписался на %s\n", pid, topic);

        } else if (strcmp(cmd, "unsubscribe") == 0) {
            for (int i = 0; i < sub_count; i++) {
                if (subs[i].pid == pid) {
                    subs[i] = subs[sub_count - 1];
                    sub_count--;
                    break;
                }
            }
            printf("[Брокер] Подписчик %d отписался\n", pid);

        } else if (strcmp(cmd, "send") == 0) {
            int found = 0;
            for (int i = 0; i < pub_count; i++) if (publishers[i] == pid) found = 1;
            if (!found && pub_count < 50) publishers[pub_count++] = pid;

            printf("[Брокер] Издатель %d отправил данные в тему %s\n", pid, topic);

            for (int i = 0; i < sub_count; i++) {
                for (int j = 0; j < subs[i].topic_count; j++) {
                    if (strcmp(subs[i].topics[j], topic) == 0) {
                        send_message(g_msqid, subs[i].pid, msg.mtext);
                        printf("[Брокер] -> Переслано подписчику %d\n", subs[i].pid);
                    }
                }
            }
        }
    }
}