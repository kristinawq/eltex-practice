#include "../calc.h"
#include <math.h>

const char *get_op_name(void) { return "sqrt"; }
const char *get_op_descr(void) { return "корень"; }
int get_op_argc(void) { return 1; }

int op_execute(double a, double b, double *r)
{
    (void)b;
    if (a < 0.0) return CALC_ERR_DOMAIN;
    *r = sqrt(a);
    return CALC_OK;
}
