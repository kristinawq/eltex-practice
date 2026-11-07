#ifndef LOGIC_H
#define LOGIC_H

#include <sys/types.h>

int parse_permissions(const char *str, unsigned int *out_mode);

void mode_to_string(unsigned int mode, char *out_str);

void mode_to_bits(unsigned int mode, char *out_str);

int apply_chmod(unsigned int current_mode, const char *cmd, unsigned int *out_new_mode);

#endif
