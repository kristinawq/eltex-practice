#include "calc.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int op_add(double a, double b, double *r)
{
    *r = a + b;
    if (isinf(*r) || isnan(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}

int op_sub(double a, double b, double *r)
{
    *r = a - b;
    if (isinf(*r) || isnan(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}

int op_mul(double a, double b, double *r)
{
    *r = a * b;
    if (isinf(*r) || isnan(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}

int op_div(double a, double b, double *r)
{
    if (b == 0.0) return CALC_ERR_DIV_ZERO;
    *r = a / b;
    if (isinf(*r) || isnan(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}

int op_pow(double a, double b, double *r)
{
    if (a == 0.0 && b == 0.0) return CALC_ERR_DOMAIN;
    if (a == 0.0 && b < 0.0)  return CALC_ERR_DIV_ZERO;
    *r = pow(a, b);
    if (isnan(*r)) return CALC_ERR_DOMAIN;
    if (isinf(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}

int op_mod(double a, double b, double *r)
{
    if (b == 0.0) return CALC_ERR_DIV_ZERO;
    *r = fmod(a, b);
    return CALC_OK;
}

int op_sqrt(double a, double b, double *r)
{
    (void)b;
    if (a < 0.0) return CALC_ERR_DOMAIN;
    *r = sqrt(a);
    return CALC_OK;
}

int calc_init(CalcTable *t)
{
    t->items = NULL;
    t->count = 0;
    t->capacity = 0;
    return CALC_OK;
}

void calc_free(CalcTable *t)
{
    free(t->items);
    t->items = NULL;
    t->count = 0;
    t->capacity = 0;
}

int calc_register(CalcTable *t, const char *name, const char *descr,
                  int argc, calc_op_fn fn)
{
    if (t->count == t->capacity) {
        int          new_cap = (t->capacity == 0) ? 4 : t->capacity * 2;
        CalcCommand *p = realloc(t->items, (size_t)new_cap * sizeof(CalcCommand));
        if (!p) return CALC_ERR_NO_MEMORY;
        t->items = p;
        t->capacity = new_cap;
    }
    strncpy(t->items[t->count].name,  name,  sizeof(t->items[0].name)  - 1);
    t->items[t->count].name[sizeof(t->items[0].name) - 1] = '\0';
    strncpy(t->items[t->count].descr, descr, sizeof(t->items[0].descr) - 1);
    t->items[t->count].descr[sizeof(t->items[0].descr) - 1] = '\0';
    t->items[t->count].argc = argc;
    t->items[t->count].fn   = fn;
    t->count++;
    return CALC_OK;
}

const CalcCommand *calc_find(const CalcTable *t, const char *name)
{
    int i;
    for (i = 0; i < t->count; ++i)
        if (strcmp(t->items[i].name, name) == 0)
            return &t->items[i];
    return NULL;
}

const char *calc_error_text(int code)
{
    switch (code) {
    case CALC_OK:            return "OK";
    case CALC_ERR_DIV_ZERO:  return "деление на ноль";
    case CALC_ERR_DOMAIN:    return "ошибка области определения";
    case CALC_ERR_OVERFLOW:  return "переполнение (inf/nan)";
    case CALC_ERR_NOT_FOUND: return "команда не найдена";
    case CALC_ERR_NO_MEMORY: return "нет памяти";
    default:                 return "неизвестная ошибка";
    }
}
