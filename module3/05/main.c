#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "producer_consumer.h"

#define DEFAULT_SHM_SIZE   1024
#define PRODUCE_PERIOD_US  300000
#define WAIT_PERIOD_US     500000
#define CONSUME_PERIOD_US  700000

static void print_values(const int *values, int count)
{
    for (int i = 0; i < count; i++)
        printf(" %d", values[i]);
}

static void find_min_max(const int *values, int count, int *min, int *max)
{
    *min = *max = values[0];
    for (int i = 1; i < count; i++) {
        if (values[i] < *min)
            *min = values[i];
        if (values[i] > *max)
            *max = values[i];
    }
}

static int run_producer(size_t size)
{
    Pc pc;
    int values[PC_MAX_VALUES];
    int total = 0;
    int rc;

    if (pc_create(&pc, PC_NAME, size) == PC_ERROR) {
        perror("Не удалось создать разделяемую память и семафор");
        if (errno == EEXIST)
            fprintf(stderr, "Остались от прошлого запуска? Удалите: ./prodcons clean\n");
        return 1;
    }
    printf("[producer] память %zu байт и семафор созданы\n", size);

    for (;;) {
        int count = 1 + rand() % PC_MAX_VALUES;

        for (int i = 0; i < count; i++)
            values[i] = rand() % 201 - 100;
        rc = pc_put(&pc, values, count);
        if (rc != PC_OK)
            break;
        total++;
        printf("[producer] записан набор:");
        print_values(values, count);
        printf("\n");
        usleep(PRODUCE_PERIOD_US);
    }
    if (rc == PC_ERROR || pc_finish(&pc) == PC_ERROR) {
        perror("Ошибка производителя");
        pc_close(&pc, 1);
        return 1;
    }
    printf("[producer] память исчерпана, записано наборов: %d\n", total);

    while ((rc = pc_unprocessed(&pc)) > 0) {
        printf("[producer] необработанных наборов: %d, жду\n", rc);
        usleep(WAIT_PERIOD_US);
    }
    if (rc == PC_ERROR) {
        perror("Ошибка проверки обработки");
        pc_close(&pc, 1);
        return 1;
    }

    printf("[producer] все наборы обработаны, отключаю и удаляю память и семафор\n");
    return pc_close(&pc, 1) == PC_ERROR ? 1 : 0;
}

static int run_consumer(void)
{
    Pc pc;
    int values[PC_MAX_VALUES];
    int count, min, max;
    int pid = getpid();

    if (pc_open(&pc, PC_NAME) == PC_ERROR) {
        perror("Не удалось подключиться (запущен ли producer?)");
        return 1;
    }
    printf("[consumer %d] подключен\n", pid);

    for (;;) {
        int rc = pc_take(&pc, values, PC_MAX_VALUES, &count);

        if (rc == PC_FINISHED) {
            printf("[consumer %d] все наборы обработаны, завершаюсь\n", pid);
            break;
        }
        if (rc == PC_ERROR) {
            perror("Ошибка потребителя");
            pc_close(&pc, 0);
            return 1;
        }
        if (rc == PC_OK) {
            find_min_max(values, count, &min, &max);
            printf("[consumer %d] набор:", pid);
            print_values(values, count);
            printf(" | max=%d min=%d\n", max, min);
        }
        usleep(CONSUME_PERIOD_US);
    }
    return pc_close(&pc, 0) == PC_ERROR ? 1 : 0;
}

static int run_clean(void)
{
    if (pc_remove(PC_NAME) == PC_ERROR) {
        perror("Не удалось удалить объекты");
        return 1;
    }
    printf("Разделяемая память и семафор удалены\n");
    return 0;
}

static int parse_size(const char *text, size_t *size)
{
    char *end;

    *size = strtoul(text, &end, 10);
    return *end == '\0' && *size >= PC_MIN_SHM_SIZE && *size <= PC_MAX_SHM_SIZE;
}

static void usage(const char *program)
{
    fprintf(stderr, "Использование:\n"
                    "  %s producer [размер_памяти_в_байтах]   (по умолчанию %d)\n"
                    "  %s consumer\n"
                    "  %s clean\n", program, DEFAULT_SHM_SIZE, program, program);
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    if (argc == 2 && strcmp(argv[1], "consumer") == 0)
        return run_consumer();
    if (argc == 2 && strcmp(argv[1], "clean") == 0)
        return run_clean();
    if ((argc == 2 || argc == 3) && strcmp(argv[1], "producer") == 0) {
        size_t size = DEFAULT_SHM_SIZE;

        if (argc == 3 && !parse_size(argv[2], &size)) {
            fprintf(stderr, "Размер памяти: от %d до %d байт\n", PC_MIN_SHM_SIZE, PC_MAX_SHM_SIZE);
            return 1;
        }
        return run_producer(size);
    }

    usage(argv[0]);
    return 1;
}
