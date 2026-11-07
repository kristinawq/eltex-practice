#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "logic.h"

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Генерация случайного 32-битного IP
static uint32_t generate_random_ip(void) {
    uint32_t ip = 0;
    ip |= (rand() % 256) << 24;
    ip |= (rand() % 256) << 16;
    ip |= (rand() % 256) << 8;
    ip |= (rand() % 256);
    return ip;
}

int main(void) {
    char ip_str[32], mask_str[32];
    int n_packets;
    uint32_t gw_ip, mask, packet_ip;
    int my_packets = 0;


    while (1) {
        printf("\nВведите IP шлюза (например, 192.168.1.10) или 'exit' для выхода: ");
        if (scanf("%31s", ip_str) != 1) {
            clear_input();
            continue;
        }
        if (ip_str[0] == 'e' && ip_str[1] == 'x') break; 
        
        if (parse_ip(ip_str, &gw_ip)) {
            break; 
        }
        clear_input();
        printf("Ошибка: неверный IP.\n");
    }

    while (1) {
        printf("Введите маску (например, 255.255.255.0 или /24): ");
        if (scanf("%31s", mask_str) != 1) {
            clear_input();
            continue;
        }
        if (parse_mask(mask_str, &mask)) {
            break; 
        }
        clear_input();
        printf("Ошибка: неверная маска.\n");
    }

    while (1) {
        printf("Введите количество пакетов (N): ");
        if (scanf("%d", &n_packets) != 1) {
            clear_input();
            continue;
        }
        if (n_packets > 0) {
            break;
        }
        clear_input();
        printf("Ошибка: количество должно быть больше нуля.\n");
    }

    // Инициализация генератора текущим временем
    srand(time(NULL)); 

    my_packets = 0;

    for (int i = 0; i < n_packets; i++) {
        packet_ip = generate_random_ip();
        if (is_same_subnet(packet_ip, gw_ip, mask)) {
            my_packets++;
        }
    }

    int other_packets = n_packets - my_packets;

    printf("\nРезультаты обработки %d пакетов:\n", n_packets);
    printf("Своя подсеть: %d шт. (%.4f%%)\n", my_packets, (float)my_packets / n_packets * 100.0f);
    printf("Другие сети:  %d шт. (%.4f%%)\n", other_packets, (float)other_packets / n_packets * 100.0f);
    
    double theoretical = get_theoretical_probability(mask);
    printf("Теоретическая вероятность: %.6f%%\n", theoretical * 100.0);
    
    return 0;
}
