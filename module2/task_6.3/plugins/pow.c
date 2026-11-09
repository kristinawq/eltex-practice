#include "../calc.h"
#include <math.h>

const char *get_op_name(void) { return "^"; }
const char *get_op_descr(void) { return "степень"; }
int get_op_argc(void) { return 2; }

int op_execute(double a, double b, double *r)
{
    if (a == 0.0 && b == 0.0) return CALC_ERR_DOMAIN;
    if (a == 0.0 && b < 0.0)  return CALC_ERR_DIV_ZERO;
    *r = pow(a, b);
    if (isnan(*r)) return CALC_ERR_DOMAIN;
    if (isinf(*r)) return CALC_ERR_OVERFLOW;
    return CALC_OK;
}
