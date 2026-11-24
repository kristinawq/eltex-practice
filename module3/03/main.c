#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "chat.h"

#define POLL_TIMEOUT_MS 100
#define EXIT_COMMAND    "exit"

static volatile sig_atomic_t sigint_received;
static atomic_bool peer_finished;

static void on_sigint(int sig)
{
    (void)sig;
    sigint_received = 1;
}

static void install_sigint_handler(void)
{
    struct sigaction sa = { .sa_handler = on_sigint };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

static void *receiver(void *arg)
{
    const chat_t *chat = arg;
    char buf[CHAT_BUF_SIZE];
    int quit = 0;

    while (!quit) {
        if (chat_receive(chat, buf, &quit) < 0) {
            if (errno == EINTR)
                continue;
            perror("Ошибка приёма");
            break;
        }
        if (quit)
            printf("[собеседник завершил чат]\n");
        else
            printf("[собеседник] %s\n", buf);
    }
    atomic_store(&peer_finished, true);
    return NULL;
}

static int should_stop(void)
{
    return sigint_received || atomic_load(&peer_finished);
}

static void send_loop(const chat_t *chat)
{
    char line[CHAT_MSG_SIZE];

    while (!should_stop()) {
        struct pollfd in = { .fd = STDIN_FILENO, .events = POLLIN };
        if (poll(&in, 1, POLL_TIMEOUT_MS) <= 0)
            continue;
        if (fgets(line, sizeof line, stdin) == NULL)
            break;

        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, EXIT_COMMAND) == 0)
            break;
        if (line[0] != '\0' && chat_send(chat, line) < 0) {
            perror("Ошибка отправки");
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_очереди>\n", argv[0]);
        return 1;
    }

    chat_t chat;
    if (chat_open(&chat, argv[1]) < 0) {
        perror("Не удалось открыть очереди");
        return 1;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    install_sigint_handler();

    printf("%s очереди %s и %s. Для выхода: \"%s\" или Ctrl+C\n",
           chat.owner ? "Созданы" : "Подключен к", chat.name_1, chat.name_2, EXIT_COMMAND);

    pthread_t receiver_thread;
    int err = pthread_create(&receiver_thread, NULL, receiver, &chat);
    if (err != 0) {
        fprintf(stderr, "Не удалось создать поток приёма: %s\n", strerror(err));
        chat_close(&chat);
        return 1;
    }

    send_loop(&chat);

    if (!atomic_load(&peer_finished) && chat_send_quit(&chat) < 0)
        perror("Не удалось отправить уведомление о завершении");

    pthread_cancel(receiver_thread);
    pthread_join(receiver_thread, NULL);
    chat_close(&chat);
    return 0;
}
