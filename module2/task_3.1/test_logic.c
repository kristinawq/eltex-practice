#include <stdio.h>
#include "unity.h"
#include "logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_parse_permissions_str(void) {
    unsigned int mode;
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("rwxr-xr--", &mode));
    TEST_ASSERT_EQUAL_INT(0754, mode);
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("rwxrwxrwx", &mode));
    TEST_ASSERT_EQUAL_INT(0777, mode);
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("---------", &mode));
    TEST_ASSERT_EQUAL_INT(0000, mode);
}

void test_parse_permissions_oct(void) {
    unsigned int mode;
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("754", &mode));
    TEST_ASSERT_EQUAL_INT(0754, mode);
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("777", &mode));
    TEST_ASSERT_EQUAL_INT(0777, mode);
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("000", &mode));
    TEST_ASSERT_EQUAL_INT(0000, mode);
}

void test_parse_permissions_invalid(void) {
    unsigned int mode;
    TEST_ASSERT_EQUAL_INT(0, parse_permissions("rwx", &mode));
    TEST_ASSERT_EQUAL_INT(0, parse_permissions("rwxrwxrwxq", &mode));
    TEST_ASSERT_EQUAL_INT(0, parse_permissions("888", &mode));
    TEST_ASSERT_EQUAL_INT(0, parse_permissions("rwxr-xr-z", &mode));
    TEST_ASSERT_EQUAL_INT(0, parse_permissions("", &mode));
}

void test_mode_to_string(void) {
    char str[12];
    mode_to_string(0754, str);
    TEST_ASSERT_EQUAL_STRING("rwxr-xr--", str);
    mode_to_string(0777, str);
    TEST_ASSERT_EQUAL_STRING("rwxrwxrwx", str);
    mode_to_string(0000, str);
    TEST_ASSERT_EQUAL_STRING("---------", str);
}

void test_mode_to_bits(void) {
    char bits[20];
    mode_to_bits(0754, bits);
    TEST_ASSERT_EQUAL_STRING("111101100", bits);
    mode_to_bits(0000, bits);
    TEST_ASSERT_EQUAL_STRING("000000000", bits);
    mode_to_bits(0777, bits);
    TEST_ASSERT_EQUAL_STRING("111111111", bits);
}

void test_apply_chmod_add(void) {
    unsigned int new_mode;
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0644, "u+x", &new_mode));
    TEST_ASSERT_EQUAL_INT(0744, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0644, "a+x", &new_mode));
    TEST_ASSERT_EQUAL_INT(0755, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0644, "g+w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0664, new_mode);
}

void test_apply_chmod_remove(void) {
    unsigned int new_mode;
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "a-w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0555, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "u-x", &new_mode));
    TEST_ASSERT_EQUAL_INT(0677, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "o-r", &new_mode));
    TEST_ASSERT_EQUAL_INT(0773, new_mode);
}

void test_apply_chmod_set(void) {
    unsigned int new_mode;
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "g=r", &new_mode));
    TEST_ASSERT_EQUAL_INT(0747, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "u=", &new_mode));
    TEST_ASSERT_EQUAL_INT(0077, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "a=rw", &new_mode));
    TEST_ASSERT_EQUAL_INT(0666, new_mode);
}

void test_apply_chmod_complex(void) {
    unsigned int new_mode;
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0000, "u+x,g-w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0100, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0000, "au+w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0222, new_mode);
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0644, "u+x,go-w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0744, new_mode);
}

void test_apply_chmod_errors(void) {
    unsigned int new_mode;
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u", &new_mode));
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u*r", &new_mode));
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u+", &new_mode));
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u+z", &new_mode));
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u+x$", &new_mode));
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "z+x", &new_mode));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_permissions_str);
    RUN_TEST(test_parse_permissions_oct);
    RUN_TEST(test_parse_permissions_invalid);
    RUN_TEST(test_mode_to_string);
    RUN_TEST(test_mode_to_bits);
    RUN_TEST(test_apply_chmod_add);
    RUN_TEST(test_apply_chmod_remove);
    RUN_TEST(test_apply_chmod_set);
    RUN_TEST(test_apply_chmod_complex);
    RUN_TEST(test_apply_chmod_errors);
    return UNITY_END();
}
