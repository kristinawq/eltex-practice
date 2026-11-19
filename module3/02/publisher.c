#include "broker.h"

void run_publisher(pid_t pid, const char *topic) {
    int msqid = get_msg_queue(0666 | IPC_CREAT);
    if (msqid < 0) {
        perror("msgget");
        return;
    }

    printf("[Издатель %d] Начал работу. Тема: %s\n", pid, topic);

    for (int i = 0; i < 5; i++) {
        char text[MAX_PAYLOAD];
        snprintf(text, sizeof(text), "send,%d,%s", pid, topic);
        
        if (send_message(msqid, 1, text) < 0) {
            perror("msgsnd");
            break;
        }
        
        printf("[Издатель %d] Отправлено сообщение в тему %s\n", pid, topic);
        sleep(1);
    }

    printf("[Издатель %d] Завершил работу\n", pid);
}