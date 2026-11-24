#include "producer_consumer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define PC_MODE 0600

static int pc_lock(const Pc *pc)
{
    while (sem_wait(pc->sem) == -1) {
        if (errno != EINTR)
            return PC_ERROR;
    }
    return PC_OK;
}

static int pc_unlock(const Pc *pc)
{
    return sem_post(pc->sem) == -1 ? PC_ERROR : PC_OK;
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

static int make_names(Pc *pc, const char *name)
{
    const char *slash;
    int n1, n2;

    if (name == NULL || name[0] == '\0')
        return PC_ERROR;
    slash = (name[0] == '/') ? "" : "/";
    n1 = snprintf(pc->shm_name, PC_NAME_SIZE, "%s%s_shm", slash, name);
    n2 = snprintf(pc->sem_name, PC_NAME_SIZE, "%s%s_sem", slash, name);
    return (n1 >= PC_NAME_SIZE || n2 >= PC_NAME_SIZE) ? PC_ERROR : PC_OK;
}

static void reset(Pc *pc)
{
    pc->shm_fd = -1;
    pc->sem = NULL;
    pc->mem = NULL;
    pc->size = 0;
}

int pc_close(Pc *pc, int destroy)
{
    int rc = PC_OK;
    int had_shm, had_sem;

    if (pc == NULL) {
        errno = EINVAL;
        return PC_ERROR;
    }
    had_shm = (pc->shm_fd != -1);
    had_sem = (pc->sem != NULL);

    if (pc->mem != NULL && munmap(pc->mem, pc->size) == -1)
        rc = PC_ERROR;
    if (had_shm && close(pc->shm_fd) == -1)
        rc = PC_ERROR;
    if (had_sem && sem_close(pc->sem) == -1)
        rc = PC_ERROR;
    if (destroy) {
        if (had_shm && shm_unlink(pc->shm_name) == -1)
            rc = PC_ERROR;
        if (had_sem && sem_unlink(pc->sem_name) == -1)
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

int pc_create(Pc *pc, const char *name, size_t size)
{
    if (pc == NULL || size < PC_MIN_SHM_SIZE || size > PC_MAX_SHM_SIZE ||
        make_names(pc, name) == PC_ERROR) {
        errno = EINVAL;
        return PC_ERROR;
    }
    reset(pc);

    pc->shm_fd = shm_open(pc->shm_name, O_CREAT | O_EXCL | O_RDWR, PC_MODE);
    if (pc->shm_fd == -1)
        return PC_ERROR;
    if (ftruncate(pc->shm_fd, (off_t)size) == -1)
        return fail_close(pc, 1);
    pc->mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, pc->shm_fd, 0);
    if (pc->mem == MAP_FAILED) {
        pc->mem = NULL;
        return fail_close(pc, 1);
    }
    pc->size = size;

    memset(pc->mem, 0, size);
    header(pc)->end = sizeof(PcHeader);

    pc->sem = sem_open(pc->sem_name, O_CREAT | O_EXCL | O_RDWR, PC_MODE, 1);
    if (pc->sem == SEM_FAILED) {
        pc->sem = NULL;
        return fail_close(pc, 1);
    }
    return PC_OK;
}

int pc_open(Pc *pc, const char *name)
{
    struct stat info;

    if (pc == NULL || make_names(pc, name) == PC_ERROR) {
        errno = EINVAL;
        return PC_ERROR;
    }
    reset(pc);

    pc->sem = sem_open(pc->sem_name, 0);
    if (pc->sem == SEM_FAILED) {
        pc->sem = NULL;
        return PC_ERROR;
    }
    pc->shm_fd = shm_open(pc->shm_name, O_RDWR, 0);
    if (pc->shm_fd == -1 || fstat(pc->shm_fd, &info) == -1)
        return fail_close(pc, 0);
    if ((size_t)info.st_size < sizeof(PcHeader)) {
        errno = EPROTO;
        return fail_close(pc, 0);
    }
    pc->mem = mmap(NULL, (size_t)info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, pc->shm_fd, 0);
    if (pc->mem == MAP_FAILED) {
        pc->mem = NULL;
        return fail_close(pc, 0);
    }
    pc->size = (size_t)info.st_size;
    return PC_OK;
}

int pc_remove(const char *name)
{
    Pc pc;
    int rc = PC_OK;

    if (make_names(&pc, name) == PC_ERROR) {
        errno = EINVAL;
        return PC_ERROR;
    }
    if (shm_unlink(pc.shm_name) == -1 && errno != ENOENT)
        rc = PC_ERROR;
    if (sem_unlink(pc.sem_name) == -1 && errno != ENOENT)
        rc = PC_ERROR;
    return rc;
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
