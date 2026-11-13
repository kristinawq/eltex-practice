#include "unity.h"
#include "copy_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void setUp(void) {}
void tearDown(void) {}

void test_get_file_size_ok(void)
{
    const char *path = "/tmp/unity_size.txt";
    FILE *f = fopen(path, "wb");
    fwrite("hello", 1, 5, f);
    fclose(f);

    TEST_ASSERT_EQUAL_INT(5, get_file_size(path));
    unlink(path);
}

void test_get_file_size_missing(void)
{
    /* Ошибка ввода: файла не существует */
    TEST_ASSERT_EQUAL_INT(-1, get_file_size("/tmp/no_such_xyz_123"));
}

void test_make_copy_name_ok(void)
{
    char dst[256];
    TEST_ASSERT_EQUAL_INT(0, make_copy_name("data.txt", dst, sizeof(dst)));
    TEST_ASSERT_EQUAL_STRING("data.txt.copy", dst);
}

void test_make_copy_name_too_small(void)
{
    /* Ошибка ввода: слишком длинное имя для буфера */
    char dst[5];
    TEST_ASSERT_EQUAL_INT(-1, make_copy_name("longname.txt", dst, sizeof(dst)));
}

void test_send_receive_roundtrip(void)
{
    int p[2];
    pipe(p);

    const char *src = "/tmp/unity_rt_src.txt";
    const char *dst = "/tmp/unity_rt_dst.txt";
    const char *content = "abcdefgh";
    long len = (long)strlen(content);

    FILE *f = fopen(src, "wb");
    fwrite(content, 1, (size_t)len, f);
    fclose(f);

    FILE *in  = fopen(src, "rb");
    FILE *out = fopen(dst, "wb");

    TEST_ASSERT_EQUAL_INT(COPY_OK, parent_send_file(in, p[1], src, len));
    fclose(in);
    close(p[1]);

    FileMeta meta;
    read(p[0], &meta, sizeof(meta));
    TEST_ASSERT_EQUAL_INT(len, meta.filesize);
    TEST_ASSERT_EQUAL_STRING(src, meta.filename);

    TEST_ASSERT_EQUAL_INT(COPY_OK, child_receive_file(p[0], out, meta.filesize));
    fclose(out);
    close(p[0]);

    char buf[32] = {0};
    f = fopen(dst, "rb");
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    TEST_ASSERT_EQUAL_STRING(content, buf);

    unlink(src); unlink(dst);
}

void test_full_fork_pipe(void)
{
    const char *src = "/tmp/unity_fork_src.txt";
    FILE *f = fopen(src, "wb");
    fwrite("fork-data", 1, 9, f);
    fclose(f);

    int p[2];
    pipe(p);

    pid_t pid = fork();
    if (pid == 0) {
        close(p[1]);
        FileMeta meta;
        read(p[0], &meta, sizeof(meta));
        char dst[512];
        make_copy_name(meta.filename, dst, sizeof(dst));
        FILE *out = fopen(dst, "wb");
        child_receive_file(p[0], out, meta.filesize);
        fclose(out);
        close(p[0]);
        _exit(0);
    }

    close(p[0]);
    long size = get_file_size(src);
    FILE *in = fopen(src, "rb");
    parent_send_file(in, p[1], src, size);
    fclose(in);
    close(p[1]);
    wait(NULL);

    char expected[512];
    make_copy_name(src, expected, sizeof(expected));
    char buf[32] = {0};
    f = fopen(expected, "rb");
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    TEST_ASSERT_EQUAL_STRING("fork-data", buf);

    unlink(src); unlink(expected);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_get_file_size_ok);
    RUN_TEST(test_get_file_size_missing);
    RUN_TEST(test_make_copy_name_ok);
    RUN_TEST(test_make_copy_name_too_small);
    RUN_TEST(test_send_receive_roundtrip);
    RUN_TEST(test_full_fork_pipe);
    return UNITY_END();
}
