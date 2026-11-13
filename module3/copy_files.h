#ifndef COPY_FILES_H
#define COPY_FILES_H

#include <stdio.h>
#include <stddef.h>

#define BUF_SIZE 4096
#define READY_MSG "READY"

typedef enum {
    COPY_OK = 0,
    COPY_ERR_OPEN_SRC,
    COPY_ERR_OPEN_DST,
    COPY_ERR_READ,
    COPY_ERR_WRITE
} CopyStatus;

typedef struct {
    char  filename[256];
    long  filesize;
} FileMeta;

long get_file_size(const char *path);
int make_copy_name(const char *src, char *dst, size_t dstlen);

CopyStatus parent_send_file(FILE *src, int pipe_fd, const char *filename, long filesize);
CopyStatus child_receive_file(int pipe_fd, FILE *dst, long filesize);

#endif
