#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>

int parse_ip(const char *str, uint32_t *out_ip);

int parse_mask(const char *str, uint32_t *out_mask);

int is_same_subnet(uint32_t packet_ip, uint32_t gateway_ip, uint32_t mask);

double get_theoretical_probability(uint32_t mask);

#endif
