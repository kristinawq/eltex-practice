#include "../calc.h"
#include <math.h>

const char *get_op_name(void) { return "/"; }
const char *get_op_descr(void) { return "деление"; }
int get_op_argc(void) { return 2; }

int op_execute(double a, double b, double *r)
{
    if (b == 0.0) return CALC_ERR_DIV_ZERO;
    *r = a / b;
    if (isinf(*r) || isnan(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}
