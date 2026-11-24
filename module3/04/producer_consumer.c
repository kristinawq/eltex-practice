#include "producer_consumer.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define PC_MODE 0600

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

static int semop_retry(int sem_id, struct sembuf *ops, size_t count)
{
    while (semop(sem_id, ops, count) == -1) {
        if (errno != EINTR)
            return PC_ERROR;
    }
    return PC_OK;
}

static int pc_lock(const Pc *pc)
{
    struct sembuf lock = {0, -1, 0};
    return semop_retry(pc->sem_id, &lock, 1);
}

static int pc_unlock(const Pc *pc)
{
    struct sembuf unlock[2] = {{0, 0, 0},
                               {0, 1, 0}};
    return semop_retry(pc->sem_id, unlock, 2);
}

static int unlock_and_return(const Pc *pc, int rc)
{
    int saved = errno;

    if (pc_unlock(pc) == PC_ERROR)
        return PC_ERROR;
    errno = saved;
    return rc;
}

static PcHeader *header(const Pc *pc)
{
    return (PcHeader *)pc->mem;
}

static PcBlock *block_at(const Pc *pc, int offset)
{
    if (offset < (int)sizeof(PcHeader) || (size_t)offset + sizeof(PcBlock) > pc->size)
        return NULL;
    return (PcBlock *)(pc->mem + offset);
}

static int scan(const Pc *pc, PcBlock **first)
{
    size_t visited_limit = pc->size / sizeof(PcBlock);
    int unprocessed = 0;
    int offset = header(pc)->first;

    *first = NULL;
    for (size_t visited = 0; offset != 0; visited++) {
        PcBlock *block = block_at(pc, offset);

        if (block == NULL || visited > visited_limit ||
            block->count < 0 || block->count > PC_MAX_VALUES) {
            errno = EPROTO;
            return PC_ERROR;
        }
        if (block->count > 0) {
            unprocessed++;
            if (*first == NULL)
                *first = block;
        }
        offset = block->next;
    }
    return unprocessed;
}

static int get_keys(const char *path, key_t *shm_key, key_t *sem_key)
{
    *shm_key = ftok(path, 'S');
    *sem_key = ftok(path, 'T');
    return (*shm_key == (key_t)-1 || *sem_key == (key_t)-1) ? PC_ERROR : PC_OK;
}

static void reset(Pc *pc)
{
    memset(pc, 0, sizeof(*pc));
    pc->shm_id = -1;
    pc->sem_id = -1;
}

int pc_close(Pc *pc, int destroy)
{
    int rc = PC_OK;

    if (pc == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (pc->mem != NULL && shmdt(pc->mem) == -1)
        rc = PC_ERROR;
    if (destroy) {
        if (pc->shm_id != -1 && shmctl(pc->shm_id, IPC_RMID, NULL) == -1)
            rc = PC_ERROR;
        if (pc->sem_id != -1 && semctl(pc->sem_id, 0, IPC_RMID) == -1)
            rc = PC_ERROR;
    }
    reset(pc);
    return rc;
}

static int fail_close(Pc *pc, int destroy)
{
    int saved = errno;

    pc_close(pc, destroy);
    errno = saved;
    return PC_ERROR;
}

int pc_create(Pc *pc, const char *key_path, size_t size)
{
    key_t shm_key, sem_key;
    union semun arg = { .val = 1 };
    int fd;

    if (pc == NULL || key_path == NULL || size < PC_MIN_SHM_SIZE || size > PC_MAX_SHM_SIZE) {
        errno = EINVAL;
        return PC_ERROR;
    }
    fd = open(key_path, O_CREAT | O_RDWR, PC_MODE);
    if (fd == -1)
        return PC_ERROR;
    close(fd);
    if (get_keys(key_path, &shm_key, &sem_key) == PC_ERROR)
        return PC_ERROR;

    reset(pc);
    pc->shm_id = shmget(shm_key, size, IPC_CREAT | IPC_EXCL | PC_MODE);
    if (pc->shm_id == -1)
        return PC_ERROR;
    pc->mem = shmat(pc->shm_id, NULL, 0);
    if (pc->mem == (char *)-1) {
        pc->mem = NULL;
        return fail_close(pc, 1);
    }
    pc->size = size;

    memset(pc->mem, 0, size);
    header(pc)->end = sizeof(PcHeader);

    pc->sem_id = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | PC_MODE);
    if (pc->sem_id == -1 || semctl(pc->sem_id, 0, SETVAL, arg) == -1)
        return fail_close(pc, 1);
    return PC_OK;
}

int pc_open(Pc *pc, const char *key_path)
{
    key_t shm_key, sem_key;
    struct shmid_ds info;

    if (pc == NULL || key_path == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (get_keys(key_path, &shm_key, &sem_key) == PC_ERROR)
        return PC_ERROR;

    reset(pc);
    pc->shm_id = shmget(shm_key, 0, 0);
    if (pc->shm_id == -1)
        return PC_ERROR;
    pc->sem_id = semget(sem_key, 0, 0);
    if (pc->sem_id == -1 || shmctl(pc->shm_id, IPC_STAT, &info) == -1)
        return fail_close(pc, 0);
    pc->mem = shmat(pc->shm_id, NULL, 0);
    if (pc->mem == (char *)-1) {
        pc->mem = NULL;
        return fail_close(pc, 0);
    }
    pc->size = info.shm_segsz;
    return PC_OK;
}

int pc_remove(const char *key_path)
{
    key_t shm_key, sem_key;
    int id;

    if (key_path == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (get_keys(key_path, &shm_key, &sem_key) == PC_ERROR)
        return (errno == ENOENT) ? PC_OK : PC_ERROR;

    id = shmget(shm_key, 0, 0);
    if (id != -1 && shmctl(id, IPC_RMID, NULL) == -1)
        return PC_ERROR;
    id = semget(sem_key, 0, 0);
    if (id != -1 && semctl(id, 0, IPC_RMID) == -1)
        return PC_ERROR;
    return PC_OK;
}

int pc_put(Pc *pc, const int *values, int count)
{
    PcHeader *h;
    PcBlock *block;
    size_t need = sizeof(PcBlock) + (size_t)count * sizeof(int);

    if (pc == NULL || values == NULL || count < 1 || count > PC_MAX_VALUES) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (pc_lock(pc) == PC_ERROR)
        return PC_ERROR;

    h = header(pc);
    if ((size_t)h->end + need > pc->size)
        return unlock_and_return(pc, PC_FULL);

    block = (PcBlock *)(pc->mem + h->end);
    block->count = count;
    block->next = 0;
    memcpy(block->values, values, (size_t)count * sizeof(int));

    if (h->last == 0)
        h->first = h->end;
    else
        ((PcBlock *)(pc->mem + h->last))->next = h->end;
    h->last = h->end;
    h->end += (int)need;
    return unlock_and_return(pc, PC_OK);
}

int pc_finish(Pc *pc)
{
    if (pc == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (pc_lock(pc) == PC_ERROR)
        return PC_ERROR;
    header(pc)->finished = 1;
    return unlock_and_return(pc, PC_OK);
}

int pc_unprocessed(Pc *pc)
{
    PcBlock *first;
    int unprocessed;

    if (pc == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (pc_lock(pc) == PC_ERROR)
        return PC_ERROR;
    unprocessed = scan(pc, &first);
    return unlock_and_return(pc, unprocessed);
}

int pc_take(Pc *pc, int *values, int max, int *count)
{
    PcBlock *block;

    if (pc == NULL || values == NULL || count == NULL || max < 1) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (pc_lock(pc) == PC_ERROR)
        return PC_ERROR;

    if (scan(pc, &block) == PC_ERROR)
        return unlock_and_return(pc, PC_ERROR);
    if (block == NULL)
        return unlock_and_return(pc, header(pc)->finished ? PC_FINISHED : PC_EMPTY);
    if (block->count > max) {
        errno = EMSGSIZE;
        return unlock_and_return(pc, PC_ERROR);
    }

    memcpy(values, block->values, (size_t)block->count * sizeof(int));
    *count = block->count;
    block->count = 0;
    return unlock_and_return(pc, PC_OK);
}
