#include "broker.h"

int g_msqid = -1;

static void print_usage(const char *prog) {
    printf("Использование:\n");
    printf("  %s -b                    (Запуск брокера)\n", prog);
    printf("  %s -p <topic>            (Запуск издателя для темы)\n", prog);
    printf("  %s -s <topic1> <topic2>  (Запуск подписчика на темы)\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    pid_t pid = getpid();

    if (strcmp(argv[1], "-b") == 0) {
        if (argc != 2) { 
            print_usage(argv[0]); 
            return 1; 
        }
        run_broker();
    } 
    else if (strcmp(argv[1], "-p") == 0) {
        if (argc != 3) { 
            print_usage(argv[0]); 
            return 1; 
        }
        run_publisher(pid, argv[2]);
    } 
    else if (strcmp(argv[1], "-s") == 0) {
        if (argc < 3) { 
            print_usage(argv[0]); 
            return 1; 
        }
        run_subscriber(pid, argc - 2, &argv[2]);
    } 
    else {
        printf("Ошибка: неизвестный флаг '%s'\n\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}