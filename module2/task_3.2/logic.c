#include "logic.h"
#include <stdio.h>
#include <string.h>

int parse_ip(const char *str, uint32_t *out_ip) {
    unsigned int a, b, c, d;
    char extra;
    
    if (sscanf(str, "%u.%u.%u.%u%c", &a, &b, &c, &d, &extra) != 4) return 0;
    
    if (a > 255 || b > 255 || c > 255 || d > 255) return 0;
    
    *out_ip = (a << 24) | (b << 16) | (c << 8) | d;
    return 1;
}

int parse_mask(const char *str, uint32_t *out_mask) {
    if (strchr(str, '.') != NULL) {
        return parse_ip(str, out_mask);
    }
    
    if (str[0] == '/') {
        int prefix = 0;
        if (sscanf(str, "/%d", &prefix) == 1) {
            if (prefix < 0 || prefix > 32) return 0;
            if (prefix == 0) *out_mask = 0;
            else *out_mask = 0xFFFFFFFF << (32 - prefix);
            return 1;
        }
        return 0;
    }
    
    return 0;
}

int is_same_subnet(uint32_t packet_ip, uint32_t gateway_ip, uint32_t mask) {
    return (packet_ip & mask) == (gateway_ip & mask);
}

double get_theoretical_probability(uint32_t mask) {
    int host_bits = 0;
    while ((mask & 1) == 0 && host_bits < 32) {
        host_bits++;
        mask >>= 1;
    }
    
    double hosts = 1.0;
    for (int i = 0; i < host_bits; i++) hosts *= 2.0;
    
    return hosts / 4294967296.0; 
}
