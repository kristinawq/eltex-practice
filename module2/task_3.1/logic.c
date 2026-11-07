#include "logic.h"
#include <stdio.h>
#include <string.h>

int parse_permissions(const char *str, unsigned int *out_mode) {
    unsigned int mode = 0;

    if (str[0] >= '0' && str[0] <= '7') {
        if (sscanf(str, "%o", &mode) != 1) return 0;
        *out_mode = mode;
        return 1;
    }

    if (strlen(str) != 9) return 0;
    for (int i = 0; i < 9; i++) {
        if (str[i] != '-') {
            mode |= (1 << (8 - i)); 
        }
    }
    *out_mode = mode;
    return 1;
}

void mode_to_string(unsigned int mode, char *out_str) {
    const char symbols[3] = {'r', 'w', 'x'};
    for (int i = 0; i < 9; i++) {
        out_str[i] = (mode & (1 << (8 - i))) ? symbols[i % 3] : '-';
    }
    out_str[9] = '\0';
}

void mode_to_bits(unsigned int mode, char *out_str) {
    int pos = 0;
    for (int i = 0; i < 9; i++) {
        out_str[pos++] = (mode & (1 << (8 - i))) ? '1' : '0';
    }
    out_str[pos] = '\0';
}

int apply_chmod(unsigned int current_mode, const char *cmd, unsigned int *out_new_mode) {
    unsigned int new_mode = current_mode;
    const char *p = cmd;

    while (*p != '\0') {
        unsigned int target_mask = 0;

        while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
            if (*p == 'u' || *p == 'a') target_mask |= (7 << 6);
            if (*p == 'g' || *p == 'a') target_mask |= (7 << 3);
            if (*p == 'o' || *p == 'a') target_mask |= (7 << 0);
            p++;
        }

        if (target_mask == 0) target_mask = 0777;

        if (*p != '+' && *p != '-' && *p != '=') return 0; 
        char op = *p;
        p++;

        unsigned int perm_mask = 0;
        int found_perm = 0;
        while (*p == 'r' || *p == 'w' || *p == 'x') {
            found_perm = 1;
            if (*p == 'r') perm_mask |= 4;
            if (*p == 'w') perm_mask |= 2;
            if (*p == 'x') perm_mask |= 1;
            p++;
        }

        if (op != '=' && !found_perm) return 0; 

        unsigned int final_mask = 0;
        if (target_mask & (7 << 6)) final_mask |= (perm_mask << 6);
        if (target_mask & (7 << 3)) final_mask |= (perm_mask << 3);
        if (target_mask & (7 << 0)) final_mask |= (perm_mask << 0);

        if (op == '+') {
            new_mode |= final_mask;
        } else if (op == '-') {
            new_mode &= ~final_mask;
        } else if (op == '=') {
            unsigned int clear_mask = 0;
            if (target_mask & (7 << 6)) clear_mask |= (7 << 6);
            if (target_mask & (7 << 3)) clear_mask |= (7 << 3);
            if (target_mask & (7 << 0)) clear_mask |= (7 << 0);
            new_mode &= ~clear_mask;
            new_mode |= final_mask;
        }

        if (*p == ',') {
            p++;
        } else if (*p != '\0') {
            return 0;
        }
    }

    *out_new_mode = new_mode;
    return 1;
}
