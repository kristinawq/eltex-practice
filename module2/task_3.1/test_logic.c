#include <stdio.h>
#include "unity.h"
#include "logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_parse_permissions_str(void) {
    unsigned int mode;
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("rwxr-xr--", &mode));
    TEST_ASSERT_EQUAL_INT(0754, mode);
}

void test_parse_permissions_oct(void) {
    unsigned int mode;
    TEST_ASSERT_EQUAL_INT(1, parse_permissions("754", &mode));
    TEST_ASSERT_EQUAL_INT(0754, mode);
}

void test_mode_to_string(void) {
    char str[12];
    mode_to_string(0754, str);
    TEST_ASSERT_EQUAL_STRING("rwxr-xr--", str);
}

void test_apply_chmod(void) {
    unsigned int new_mode;
    
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0644, "u+x", &new_mode));
    TEST_ASSERT_EQUAL_INT(0744, new_mode);

    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "a-w", &new_mode));
    TEST_ASSERT_EQUAL_INT(0555, new_mode);
    
    TEST_ASSERT_EQUAL_INT(1, apply_chmod(0777, "g=r", &new_mode));
    TEST_ASSERT_EQUAL_INT(0747, new_mode);

    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "x", &new_mode)); // мало символов
    TEST_ASSERT_EQUAL_INT(0, apply_chmod(0644, "u+z", &new_mode)); // неверное право
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_permissions_str);
    RUN_TEST(test_parse_permissions_oct);
    RUN_TEST(test_mode_to_string);
    RUN_TEST(test_apply_chmod);
    return UNITY_END();
}
