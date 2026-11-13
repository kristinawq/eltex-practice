#include "copy_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

static void usage(const char *prog)
{
    fprintf(stderr, "Использование: %s [-p имя_канала] файл1 [файл2 ...]\n", prog);
}

int main(int argc, char *argv[])
{
    const char *pipe_name = NULL;
    int start_index = 1;

    if (argc > 1 && strcmp(argv[1], "-p") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: после флага -p не указано имя канала.\n");
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        pipe_name = argv[2];
        start_index = 3;
    }

    if (start_index >= argc) {
        fprintf(stderr, "Ошибка: не указаны файлы для копирования.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int file_count = argc - start_index;
    const char **file_list = (const char **)&argv[start_index];
    int use_fifo = (pipe_name != NULL);

// создание каналов
    int data_rfd = -1, data_wfd = -1;

// канал для передачи данных
    if (use_fifo) {
        unlink(pipe_name);
        if (mkfifo(pipe_name, 0666) < 0) {
            perror("Ошибка создания именованного канала (mkfifo)");
            return EXIT_FAILURE;
        }
        data_rfd = open(pipe_name, O_RDWR);
        if (data_rfd < 0) {
            perror("Ошибка открытия именованного канала");
            unlink(pipe_name);
            return EXIT_FAILURE;
        }
        data_wfd = data_rfd;
    } else {
        int fds[2];
        if (pipe(fds) < 0) {
            perror("Ошибка создания анонимного канала (pipe)");
            return EXIT_FAILURE;
        }
        data_rfd = fds[0];
        data_wfd = fds[1];
    }

// канал для сигнала готовности
    int ready_fds[2];
    if (pipe(ready_fds) < 0) {
        perror("Ошибка создания канала готовности");
        return EXIT_FAILURE;
    }


    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка создания дочернего процесса (fork)");
        return EXIT_FAILURE;
    }


    if (pid == 0) {
        close(ready_fds[0]);
        if (!use_fifo) close(data_wfd);

        int rc = EXIT_SUCCESS;
        for (int i = 0; i < file_count; i++) {
            if (write(ready_fds[1], READY_MSG, sizeof(READY_MSG)) < 0) {
                fprintf(stderr, "Дочерний процесс: ошибка отправки READY\n");
                rc = EXIT_FAILURE; break;
            }

            FileMeta meta;
            ssize_t n = read(data_rfd, &meta, sizeof(meta));
            if (n != (ssize_t)sizeof(meta)) { 
                fprintf(stderr, "Дочерний процесс: ошибка чтения метаданных\n");
                rc = EXIT_FAILURE; break; 
            }

            char dst[512];
            if (make_copy_name(meta.filename, dst, sizeof(dst)) < 0) {
                fprintf(stderr, "Дочерний процесс: слишком длинное имя файла\n");
                rc = EXIT_FAILURE; break;
            }

            FILE *out = fopen(dst, "wb");
            if (!out) {
                fprintf(stderr, "Дочерний процесс: не удалось создать файл %s\n", dst);
                rc = EXIT_FAILURE; break;
            }

            CopyStatus st = child_receive_file(data_rfd, out, meta.filesize);
            fclose(out);

            if (st != COPY_OK) {
                fprintf(stderr, "Дочерний процесс: ошибка приема данных для %s\n", meta.filename);
                rc = EXIT_FAILURE; break;
            }
        }
        _exit(rc);
    }

    close(ready_fds[1]);
    if (!use_fifo) close(data_rfd);

    int rc = EXIT_SUCCESS;
    for (int i = 0; i < file_count; i++) {
        const char *path = file_list[i];

        char ready[sizeof(READY_MSG)];
        ssize_t n = read(ready_fds[0], ready, sizeof(ready));
        if (n != (ssize_t)sizeof(READY_MSG)) {
            fprintf(stderr, "Родительский процесс: не дождался READY от ребенка\n");
            rc = EXIT_FAILURE; break;
        }

        long size = get_file_size(path);
        if (size < 0) {
            fprintf(stderr, "Ошибка: не удалось открыть файл '%s': %s\n", path, strerror(errno));
            rc = EXIT_FAILURE; 
            continue;
        }

        FILE *in = fopen(path, "rb");
        if (!in) {
            fprintf(stderr, "Ошибка: не удалось прочитать файл '%s': %s\n", path, strerror(errno));
            rc = EXIT_FAILURE; continue;
        }

        CopyStatus st = parent_send_file(in, data_wfd, path, size);
        fclose(in);

        if (st != COPY_OK) {
            fprintf(stderr, "Родительский процесс: ошибка отправки файла %s\n", path);
            rc = EXIT_FAILURE;
        }
    }


    close(data_wfd);
    close(ready_fds[0]);

    int status;
    waitpid(pid, &status, 0);

    if (use_fifo) {
        unlink(pipe_name);
    }

    if (rc == EXIT_SUCCESS) {
        printf("Копирование успешно завершено.\n");
    }

    return rc;
}
