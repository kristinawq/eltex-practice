#include "copy_files.h"
#include <string.h>
#include <unistd.h>

long get_file_size(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long size = ftell(f);
    fclose(f);
    return size;
}

int make_copy_name(const char *src, char *dst, size_t dstlen)
{
    size_t slen = strlen(src);
    const char suffix[] = ".copy";
    size_t total = slen + sizeof(suffix);

    if (total > dstlen) return -1;

    memcpy(dst, src, slen);
    memcpy(dst + slen, suffix, sizeof(suffix));
    return 0;
}

CopyStatus parent_send_file(FILE *src, int pipe_fd,
                            const char *filename, long filesize)
{
    FileMeta meta;
    memset(&meta, 0, sizeof(meta));
    strncpy(meta.filename, filename, sizeof(meta.filename) - 1);
    meta.filesize = filesize;

    if (write(pipe_fd, &meta, sizeof(meta)) != (ssize_t)sizeof(meta))
        return COPY_ERR_WRITE;

    char buf[BUF_SIZE];
    long left = filesize;

    while (left > 0) {
        size_t want = (left < BUF_SIZE) ? (size_t)left : BUF_SIZE;
        size_t n = fread(buf, 1, want, src);
        
        if (n == 0) {
            if (ferror(src)) return COPY_ERR_READ;
            break;
        }
        if (write(pipe_fd, buf, n) != (ssize_t)n)
            return COPY_ERR_WRITE;
            
        left -= (long)n;
    }
    return COPY_OK;
}

CopyStatus child_receive_file(int pipe_fd, FILE *dst, long filesize)
{
    char buf[BUF_SIZE];
    long left = filesize;

    while (left > 0) {
        size_t want = (left < BUF_SIZE) ? (size_t)left : BUF_SIZE;
        ssize_t n = read(pipe_fd, buf, want);
        
        if (n < 0) return COPY_ERR_READ;
        if (n == 0) break;

        if (fwrite(buf, 1, (size_t)n, dst) != (size_t)n)
            return COPY_ERR_WRITE;
            
        left -= (long)n;
    }
    return COPY_OK;
}
