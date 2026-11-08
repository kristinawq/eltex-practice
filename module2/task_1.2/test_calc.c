#include <stdio.h>
#include "unity.h"
#include "calc.h"

static CalcTable t;

void setUp(void)
{
    calc_init(&t);
}

void tearDown(void)
{
    calc_free(&t);
}


void test_add(void)
{
    double r = 0.0;
    int code = op_add(2.5, 3.5, &r);
    printf("Сложение: 2.5 + 3.5 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 6.0, r);
}

void test_sub(void)
{
    double r = 0.0;
    int code = op_sub(10.0, 3.0, &r);
    printf("Вычитание: 10 - 3 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 7.0, r);
}

void test_mul(void)
{
    double r = 0.0;
    int code = op_mul(4.0, 2.5, &r);
    printf("Умножение: 4 * 2.5 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, r);
}

void test_div_ok(void)
{
    double r = 0.0;
    int code = op_div(10.0, 4.0, &r);
    printf("Деление: 10 / 4 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 2.5, r);
}

void test_div_by_zero(void)
{
    double r = 0.0;
    int code = op_div(5.0, 0.0, &r);
    printf("Деление на ноль: код ошибки %d\n", code);
    TEST_ASSERT_EQUAL_INT(CALC_ERR_DIV_ZERO, code);
}

void test_pow(void)
{
    double r = 0.0;
    int code = op_pow(2.0, 10.0, &r);
    printf("Степень: 2 ^ 10 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1024.0, r);
}

void test_pow_domain(void)
{
    double r = 0.0;
    int code1 = op_pow(0.0, 0.0, &r);
    int code2 = op_pow(0.0, -1.0, &r);
    printf("Степень (0^0): код %d\n", code1);
    printf("Степень (0^-1): код %d\n", code2);
    TEST_ASSERT_EQUAL_INT(CALC_ERR_DOMAIN, code1);
    TEST_ASSERT_EQUAL_INT(CALC_ERR_DIV_ZERO, code2);
}

void test_mod(void)
{
    double r = 0.0;
    int code = op_mod(7.5, 2.0, &r);
    printf("Остаток: 7.5 %% 2 = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.5, r);
}

void test_mod_by_zero(void)
{
    double r = 0.0;
    int code = op_mod(7.0, 0.0, &r);
    printf("Остаток на ноль: код ошибки %d\n", code);
    TEST_ASSERT_EQUAL_INT(CALC_ERR_DIV_ZERO, code);
}

void test_sqrt_ok(void)
{
    double r = 0.0;
    int code = op_sqrt(16.0, 0.0, &r);
    printf("Корень: sqrt(16) = %g\n", r);
    TEST_ASSERT_EQUAL_INT(CALC_OK, code);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 4.0, r);
}

void test_sqrt_negative(void)
{
    double r = 0.0;
    int code = op_sqrt(-4.0, 0.0, &r);
    printf("Корень из -4: код ошибки %d\n", code);
    TEST_ASSERT_EQUAL_INT(CALC_ERR_DOMAIN, code);
}


void test_register_and_find(void)
{
    TEST_ASSERT_EQUAL_INT(CALC_OK, calc_register(&t, "+", "сложение", 2, op_add));
    const CalcCommand *c = calc_find(&t, "+");
    printf("Поиск команды '+': найдена, аргументов = %d\n", c->argc);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(2, c->argc);
    TEST_ASSERT_EQUAL_STRING("+", c->name);
}

void test_register_new_command(void)
{
    TEST_ASSERT_EQUAL_INT(CALC_OK, calc_register(&t, "custom", "моя", 2, op_add));
    const CalcCommand *c = calc_find(&t, "custom");
    printf("Регистрация новой команды 'custom': найдена, аргументов = %d\n", c->argc);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(2, c->argc);
}

void test_find_not_found(void)
{
    TEST_ASSERT_EQUAL_INT(CALC_OK, calc_register(&t, "+", "сложение", 2, op_add));
    const CalcCommand *c = calc_find(&t, "abc");
    printf("Поиск несуществующей команды 'abc': указатель %s\n", c == NULL ? "NULL (не найдена)" : "не NULL");
    TEST_ASSERT_NULL(c);
}


void test_error_text(void)
{
    printf("Текст ошибки для деления на ноль: \"%s\"\n", calc_error_text(CALC_ERR_DIV_ZERO));
    TEST_ASSERT_EQUAL_STRING("OK", calc_error_text(CALC_OK));
    TEST_ASSERT_EQUAL_STRING("деление на ноль", calc_error_text(CALC_ERR_DIV_ZERO));
    TEST_ASSERT_EQUAL_STRING("ошибка области определения",
                             calc_error_text(CALC_ERR_DOMAIN));
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_add);
    RUN_TEST(test_sub);
    RUN_TEST(test_mul);
    RUN_TEST(test_div_ok);
    RUN_TEST(test_div_by_zero);
    RUN_TEST(test_pow);
    RUN_TEST(test_pow_domain);
    RUN_TEST(test_mod);
    RUN_TEST(test_mod_by_zero);
    RUN_TEST(test_sqrt_ok);
    RUN_TEST(test_sqrt_negative);

    RUN_TEST(test_register_and_find);
    RUN_TEST(test_register_new_command);
    RUN_TEST(test_find_not_found);

    RUN_TEST(test_error_text);

    return UNITY_END();
}
