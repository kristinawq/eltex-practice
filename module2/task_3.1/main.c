#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "logic.h"

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void print_mode_info(unsigned int mode) {
    char str_mode[12];
    char bit_mode[20];
    mode_to_string(mode, str_mode);
    mode_to_bits(mode, bit_mode);
    printf("Буквенно: %s | Цифровое: %o | Битовое: %s\n", str_mode, mode, bit_mode);
}

int main(void) {
    int choice;
    char input[64];
    unsigned int mode = 0;
    
    char filename[256];
    struct stat file_stat;
    int has_file_mode = 0; 

    while (1) {
        printf("\n");
        printf("1. Ввести права\n");
        printf("2. Узнать права файла\n");
        printf("3. Изменить права\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input();
            continue; 
        }
        clear_input(); 

        if (choice == 0) break;

        if (choice == 1) {
            printf("Введите права ('rwxr-xr--' или '754'): ");
            if (scanf("%63s", input) != 1) { clear_input(); continue; }
            if (parse_permissions(input, &mode)) {
                print_mode_info(mode);
                has_file_mode = 1;
            } else {
                printf("Ошибка: неверный формат.\n");
            }
        } 
        else if (choice == 2) {
            printf("Введите имя файла: ");
            if (scanf("%255s", filename) != 1) { clear_input(); continue; }
            
            if (stat(filename, &file_stat) == 0) {
                mode = file_stat.st_mode & 0777;
                printf("Файл найден. Права:\n");
                print_mode_info(mode);
                has_file_mode = 1;
            } else {
                printf("Ошибка: файл '%s' не найден.\n", filename);
            }
        } 
        else if (choice == 3) {
            if (!has_file_mode) {
                printf("Сначала введите права или укажите файл\n");
                continue;
            }
            printf("Введите команду: ");
            if (scanf("%63s", input) != 1) { clear_input(); continue; }
            
            unsigned int new_mode;
            if (apply_chmod(mode, input, &new_mode)) {
                printf("До:   ");
                print_mode_info(mode);
                printf("После: ");
                print_mode_info(new_mode);
                mode = new_mode;
            } else {
                printf("Ошибка: неверный формат команды.\n");
            }
        }
    }
    return 0;
}
