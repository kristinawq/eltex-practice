#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <stddef.h>

#define PC_KEY_PATH     "/tmp/pc_04.key"
#define PC_MAX_VALUES   16
#define PC_MIN_SHM_SIZE 64
#define PC_MAX_SHM_SIZE 65536

#define PC_OK        0
#define PC_ERROR    (-1)
#define PC_FULL      1
#define PC_EMPTY     1
#define PC_FINISHED  2

typedef struct {
    int first;
    int last;
    int end;
    int finished;
} PcHeader;

typedef struct {
    int count;
    int next;
    int values[];
} PcBlock;

typedef struct {
    int shm_id;
    int sem_id;
    char *mem;
    size_t size;
} Pc;

int pc_create(Pc *pc, const char *key_path, size_t size);
int pc_open(Pc *pc, const char *key_path);
int pc_close(Pc *pc, int destroy);
int pc_remove(const char *key_path);

int pc_put(Pc *pc, const int *values, int count);
int pc_finish(Pc *pc);
int pc_unprocessed(Pc *pc);
int pc_take(Pc *pc, int *values, int max, int *count);

#endif
