#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_PRECISION 1e-6
#include "unity.h"
#include "calc.h"
#include <math.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

static int dummy_op(double a, double b, double *result)
{
    *result = a + b;
    return CALC_OK;
}

static calc_op_fn load_plugin_op(const char *filename)
{
    char path[256];
    snprintf(path, sizeof(path), "plugins/%s", filename);
    
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) return NULL;
    
    calc_op_fn fn = (calc_op_fn)dlsym(handle, "op_execute");
    if (!fn) {
        dlclose(handle);
        return NULL;
    }
    return fn;
}

void setUp(void) {}
void tearDown(void) {}

void test_init_creates_empty_table(void)
{
    CalcTable t;
    TEST_ASSERT_EQUAL(CALC_OK, calc_init(&t));
    TEST_ASSERT_NULL(t.items);
    TEST_ASSERT_EQUAL_INT(0, t.count);
    TEST_ASSERT_EQUAL_INT(0, t.capacity);
    TEST_ASSERT_NULL(t.handles);
    TEST_ASSERT_EQUAL_INT(0, t.handles_count);
    calc_free(&t);
}

void test_register_adds_command(void)
{
    CalcTable t;
    calc_init(&t);
    TEST_ASSERT_EQUAL(CALC_OK, calc_register(&t, "custom", "моя функция", 2, dummy_op));
    TEST_ASSERT_EQUAL_INT(1, t.count);
    TEST_ASSERT_EQUAL_STRING("custom", t.items[0].name);
    TEST_ASSERT_EQUAL_STRING("моя функция", t.items[0].descr);
    TEST_ASSERT_EQUAL_INT(2, t.items[0].argc);
    TEST_ASSERT_EQUAL_PTR(dummy_op, t.items[0].fn);
    calc_free(&t);
}

void test_register_truncates_long_strings(void)
{
    CalcTable t;
    calc_init(&t);
    TEST_ASSERT_EQUAL(CALC_OK, calc_register(&t, "very_long_command_name", "очень длинное описание команды, которое точно не влезет в буфер", 1, dummy_op));
    TEST_ASSERT_EQUAL_INT(15, strlen(t.items[0].name));
    TEST_ASSERT_EQUAL_INT(63, strlen(t.items[0].descr));
    calc_free(&t);
}

void test_register_grows_capacity(void)
{
    CalcTable t;
    calc_init(&t);
    for (int i = 0; i < 20; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "cmd_%d", i);
        TEST_ASSERT_EQUAL(CALC_OK, calc_register(&t, name, "test", 1, dummy_op));
    }
    TEST_ASSERT_EQUAL_INT(20, t.count);
    TEST_ASSERT_TRUE(t.capacity >= 20);
    calc_free(&t);
}

void test_find_existing_and_missing(void)
{
    CalcTable t;
    calc_init(&t);
    calc_register(&t, "plus", "сложение", 2, dummy_op);
    calc_register(&t, "minus", "вычитание", 2, dummy_op);
    const CalcCommand *found = calc_find(&t, "plus");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STRING("plus", found->name);
    TEST_ASSERT_NULL(calc_find(&t, "multiply"));
    TEST_ASSERT_NULL(calc_find(&t, ""));
    calc_free(&t);
}

void test_error_text_returns_non_empty(void)
{
    int codes[] = { CALC_OK, CALC_ERR_DIV_ZERO, CALC_ERR_DOMAIN, CALC_ERR_OVERFLOW, 
                    CALC_ERR_NOT_FOUND, CALC_ERR_NO_MEMORY, CALC_ERR_DLOPEN, CALC_ERR_DLSYM, -999 };
    for (size_t i = 0; i < sizeof(codes)/sizeof(codes[0]); ++i) {
        const char *text = calc_error_text(codes[i]);
        TEST_ASSERT_NOT_NULL(text);
        TEST_ASSERT_TRUE(strlen(text) > 0);
    }
}

void test_load_plugins_invalid_dir(void)
{
    CalcTable t;
    calc_init(&t);
    TEST_ASSERT_EQUAL(CALC_ERR_DLOPEN, calc_load_plugins(&t, "non_existent_folder_12345"));
    calc_free(&t);
}

void test_load_plugins_real_dir(void)
{
    CalcTable t;
    calc_init(&t);
    
    calc_load_plugins(&t, "plugins");
    
    for (int i = 0; i < t.count; i++) {
        printf("Имя: [%s], Описание: [%s]\n", t.items[i].name, t.items[i].descr);
    }

    TEST_ASSERT_EQUAL_INT(7, t.count);
    
    TEST_ASSERT_NOT_NULL(calc_find(&t, "+"));
    
    calc_free(&t);
}
void test_plugin_add(void)
{
    calc_op_fn fn = load_plugin_op("add.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(2.5, 3.5, &r));
    TEST_ASSERT_EQUAL_DOUBLE(6.0, r);
}

void test_plugin_sub(void)
{
    calc_op_fn fn = load_plugin_op("sub.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(5.0, 3.0, &r));
    TEST_ASSERT_EQUAL_DOUBLE(2.0, r);
}

void test_plugin_mul(void)
{
    calc_op_fn fn = load_plugin_op("mul.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(4.0, 2.5, &r));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, r);
}

void test_plugin_div_ok(void)
{
    calc_op_fn fn = load_plugin_op("div.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(10.0, 4.0, &r));
    TEST_ASSERT_EQUAL_DOUBLE(2.5, r);
}

void test_plugin_div_by_zero(void)
{
    calc_op_fn fn = load_plugin_op("div.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_ERR_DIV_ZERO, fn(10.0, 0.0, &r));
}

void test_plugin_pow_ok(void)
{
    calc_op_fn fn = load_plugin_op("pow.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(2.0, 3.0, &r));
    TEST_ASSERT_EQUAL_DOUBLE(8.0, r);
}

void test_plugin_pow_domain_error(void)
{
    calc_op_fn fn = load_plugin_op("pow.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_ERR_DOMAIN, fn(0.0, 0.0, &r));
}

void test_plugin_pow_div_zero(void)
{
    calc_op_fn fn = load_plugin_op("pow.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_ERR_DIV_ZERO, fn(0.0, -1.0, &r));
}

void test_plugin_mod_ok(void)
{
    calc_op_fn fn = load_plugin_op("mod.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(10.0, 3.0, &r));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, r);
}

void test_plugin_mod_by_zero(void)
{
    calc_op_fn fn = load_plugin_op("mod.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_ERR_DIV_ZERO, fn(10.0, 0.0, &r));
}

void test_plugin_sqrt_ok(void)
{
    calc_op_fn fn = load_plugin_op("sqrt.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_OK, fn(25.0, 0.0, &r));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, r);
}

void test_plugin_sqrt_negative(void)
{
    calc_op_fn fn = load_plugin_op("sqrt.so");
    TEST_ASSERT_NOT_NULL(fn);
    double r = 0;
    TEST_ASSERT_EQUAL(CALC_ERR_DOMAIN, fn(-4.0, 0.0, &r));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_creates_empty_table);
    RUN_TEST(test_register_adds_command);
    RUN_TEST(test_register_truncates_long_strings);
    RUN_TEST(test_register_grows_capacity);
    RUN_TEST(test_find_existing_and_missing);
    RUN_TEST(test_error_text_returns_non_empty);
    RUN_TEST(test_load_plugins_invalid_dir);
    RUN_TEST(test_load_plugins_real_dir);
    RUN_TEST(test_plugin_add);
    RUN_TEST(test_plugin_sub);
    RUN_TEST(test_plugin_mul);
    RUN_TEST(test_plugin_div_ok);
    RUN_TEST(test_plugin_div_by_zero);
    RUN_TEST(test_plugin_pow_ok);
    RUN_TEST(test_plugin_pow_domain_error);
    RUN_TEST(test_plugin_pow_div_zero);
    RUN_TEST(test_plugin_mod_ok);
    RUN_TEST(test_plugin_mod_by_zero);
    RUN_TEST(test_plugin_sqrt_ok);
    RUN_TEST(test_plugin_sqrt_negative);
    return UNITY_END();
}
