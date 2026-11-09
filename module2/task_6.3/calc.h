#ifndef CALC_H
#define CALC_H

#define CALC_OK              0
#define CALC_ERR_DIV_ZERO   -1
#define CALC_ERR_DOMAIN     -2
#define CALC_ERR_OVERFLOW   -3
#define CALC_ERR_NOT_FOUND  -4
#define CALC_ERR_NO_MEMORY  -5
#define CALC_ERR_DLOPEN     -6
#define CALC_ERR_DLSYM      -7

typedef int (*calc_op_fn)(double a, double b, double *result);
typedef const char *(*calc_name_fn)(void);
typedef const char *(*calc_descr_fn)(void);
typedef int (*calc_argc_fn)(void);

typedef struct {
    char        name[16];
    char        descr[64];
    int         argc;
    calc_op_fn  fn;
} CalcCommand;

typedef struct {
    CalcCommand *items;
    int          count;
    int          capacity;
    void       **handles;
    int          handles_count;
    int          handles_capacity;
} CalcTable;

int  calc_init(CalcTable *t);
void calc_free(CalcTable *t);
int  calc_register(CalcTable *t, const char *name, const char *descr, int argc, calc_op_fn fn);
const CalcCommand *calc_find(const CalcTable *t, const char *name);
const char *calc_error_text(int code);

int calc_load_plugins(CalcTable *t, const char *dir_path);

#endif
