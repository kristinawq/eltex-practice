#include "calc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>

int calc_init(CalcTable *t)
{
    t->items = NULL;
    t->count = 0;
    t->capacity = 0;
    t->handles = NULL;
    t->handles_count = 0;
    t->handles_capacity = 0;
    return CALC_OK;
}

void calc_free(CalcTable *t)
{
    int i;
    for (i = 0; i < t->handles_count; ++i) {
        dlclose(t->handles[i]);
    }
    free(t->handles);
    free(t->items);
    t->items = NULL;
    t->count = 0;
    t->capacity = 0;
    t->handles = NULL;
    t->handles_count = 0;
    t->handles_capacity = 0;
}

int calc_register(CalcTable *t, const char *name, const char *descr, int argc, calc_op_fn fn)
{
    if (t->count == t->capacity) {
        int new_cap = (t->capacity == 0) ? 4 : t->capacity * 2;
        CalcCommand *p = realloc(t->items, (size_t)new_cap * sizeof(CalcCommand));
        if (!p) return CALC_ERR_NO_MEMORY;
        t->items = p;
        t->capacity = new_cap;
    }
    strncpy(t->items[t->count].name, name, sizeof(t->items[0].name) - 1);
    t->items[t->count].name[sizeof(t->items[0].name) - 1] = '\0';
    strncpy(t->items[t->count].descr, descr, sizeof(t->items[0].descr) - 1);
    t->items[t->count].descr[sizeof(t->items[0].descr) - 1] = '\0';
    t->items[t->count].argc = argc;
    t->items[t->count].fn = fn;
    t->count++;
    return CALC_OK;
}

static int calc_add_handle(CalcTable *t, void *handle)
{
    if (t->handles_count == t->handles_capacity) {
        int new_cap = (t->handles_capacity == 0) ? 4 : t->handles_capacity * 2;
        void **p = realloc(t->handles, (size_t)new_cap * sizeof(void *));
        if (!p) return CALC_ERR_NO_MEMORY;
        t->handles = p;
        t->handles_capacity = new_cap;
    }
    t->handles[t->handles_count++] = handle;
    return CALC_OK;
}

int calc_load_plugins(CalcTable *t, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir) return CALC_ERR_DLOPEN;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        int len = strlen(entry->d_name);
        if (len < 3 || strcmp(entry->d_name + len - 3, ".so") != 0) continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        void *handle = dlopen(path, RTLD_LAZY);
        if (!handle) continue;

        calc_name_fn get_name = (calc_name_fn)dlsym(handle, "get_op_name");
        calc_descr_fn get_descr = (calc_descr_fn)dlsym(handle, "get_op_descr");
        calc_argc_fn get_argc = (calc_argc_fn)dlsym(handle, "get_op_argc");
        calc_op_fn op_func = (calc_op_fn)dlsym(handle, "op_execute");

        if (get_name && get_descr && get_argc && op_func) {
            calc_register(t, get_name(), get_descr(), get_argc(), op_func);
            calc_add_handle(t, handle);
        } else {
            dlclose(handle);
        }
    }
    closedir(dir);
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
    case CALC_ERR_DLOPEN:    return "ошибка загрузки библиотеки";
    case CALC_ERR_DLSYM:     return "ошибка поиска функции в библиотеке";
    default:                 return "неизвестная ошибка";
    }
}
