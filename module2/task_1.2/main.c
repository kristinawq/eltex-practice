#include <stdio.h>
#include <string.h>
#include "calc.h"

static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static int read_double(double *out)
{
    if (scanf("%lf", out) != 1) {
        clear_input();
        return 0;
    }
    return 1;
}

static void print_menu(const CalcTable *t)
{
    int i;
    printf("\nДоступные команды:\n");
    for (i = 0; i < t->count; ++i)
        printf("  %-6s — %s\n", t->items[i].name, t->items[i].descr);
    printf("  %-6s — %s\n", "exit", "выход");
}

static int run_command(const CalcTable *t, const char *name)
{
    const CalcCommand *cmd;
    double a = 0.0, b = 0.0, result = 0.0;
    int code;

    if (strcmp(name, "exit") == 0)
        return 0;

    cmd = calc_find(t, name);
    if (cmd == NULL) {
        printf("Ошибка: команда \"%s\" не найдена.\n", name);
        return 1;
    }

    printf("Введите %d аргумент(а/ов): ", cmd->argc);
    if (!read_double(&a)) {
        printf("Ошибка: ожидалось число.\n");
        return 1;
    }
    if (cmd->argc == 2) {
        if (!read_double(&b)) {
            printf("Ошибка: ожидалось число.\n");
            return 1;
        }
    }

    code = cmd->fn(a, b, &result);
    if (code != CALC_OK)
        printf("Ошибка: %s\n", calc_error_text(code));
    else
        printf("Результат: %g\n", result);
    return 1;
}

int main(void)
{
    CalcTable t;
    char name[32];

    if (calc_init(&t) != CALC_OK) {
        printf("Не удалось инициализировать таблицу.\n");
        return 1;
    }

    if (calc_register(&t, "+",    "сложение",          2, op_add)  != CALC_OK ||
        calc_register(&t, "-",    "вычитание",         2, op_sub)  != CALC_OK ||
        calc_register(&t, "*",    "умножение",         2, op_mul)  != CALC_OK ||
        calc_register(&t, "/",    "деление",           2, op_div)  != CALC_OK ||
        calc_register(&t, "^",    "степень",           2, op_pow)  != CALC_OK ||
        calc_register(&t, "%",    "остаток",           2, op_mod)  != CALC_OK ||
        calc_register(&t, "sqrt", "квадратный корень", 1, op_sqrt) != CALC_OK) {
        printf("Не хватило памяти при регистрации команд.\n");
        calc_free(&t);
        return 1;
    }

    printf("Калькулятор. Введите \"exit\" для выхода.\n");

    for (;;) {
        print_menu(&t);
        printf("Команда: ");
        if (scanf("%31s", name) != 1) {
            clear_input();
            printf("\nКонец ввода.\n");
            break;
        }
        if (!run_command(&t, name))
            break;
    }

    calc_free(&t);
    return 0;
}
