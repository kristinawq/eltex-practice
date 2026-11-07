#include <stdio.h>
#include <string.h>
#include "contacts.h"

void input_string(char *buffer, int size) {
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; 
}

int input_number() {
    int num;
    while (scanf("%d", &num) != 1) {
        while (getchar() != '\n'); 
        printf("Ошибка! Введите число: ");
    }
    getchar(); 
    return num;
}

void print_contact(Contact c, int index) {
    printf("[%d] %s %s\n", index + 1, c.first_name, c.last_name);
    if (strlen(c.job) > 0) printf("    Работа: %s\n", c.job);
    if (strlen(c.position) > 0) printf("    Должность: %s\n", c.position);
    if (strlen(c.phone) > 0) printf("    Телефон: %s\n", c.phone);
    if (strlen(c.email) > 0) printf("    Email: %s\n", c.email);
    if (strlen(c.social) > 0) printf("    Соцсети: %s\n", c.social);
    if (strlen(c.messenger) > 0) printf("    Мессенджеры: %s\n", c.messenger);
    printf("\n");
}

int main() {
    Contact contacts[MAX_CONTACTS];
    int count = 0;
    int choice;

    while (1) {
        printf("\n");
        printf("1. Добавить контакт\n");
        printf("2. Показать все контакты\n");
        printf("3. Редактировать контакт\n");
        printf("4. Удалить контакт\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        
        choice = input_number();

        if (choice == 0) break;

        if (choice == 1) {
            Contact new_c = {0};
            printf("Введите имя: "); 
            input_string(new_c.first_name, 50);
            printf("Введите фамилию: "); 
            input_string(new_c.last_name, 50);
            
            if (strlen(new_c.first_name) == 0 || strlen(new_c.last_name) == 0) {
                printf("Ошибка: Имя и фамилия обязательны! Контакт не добавлен.\n");
                continue;
            }

            printf("Место работы (Enter - пропустить): "); input_string(new_c.job, 100);
            printf("Должность (Enter - пропустить): "); input_string(new_c.position, 100);
            printf("Телефон (Enter - пропустить): "); input_string(new_c.phone, 20);
            printf("Email (Enter - пропустить): "); input_string(new_c.email, 50);
            printf("Соцсети (Enter - пропустить): "); input_string(new_c.social, 100);
            printf("Мессенджеры (Enter - пропустить): "); input_string(new_c.messenger, 100);
            
            if (add_contact(contacts, &count, new_c) == 0) {
                printf("Контакт успешно добавлен!\n");
            } else {
                printf("Ошибка: книга переполнена!\n");
            }
        } 
        else if (choice == 2) {
            if (count == 0) {
                printf("Книга пуста.\n");
            } else {
                printf("\nВсего контактов: %d\n", count);
                for (int i = 0; i < count; i++) {
                    print_contact(contacts[i], i);
                }
            }
        }
        else if (choice == 3) {
            if (count == 0) { 
                printf("Книга пуста. Нечего редактировать.\n"); 
                continue; 
            }
            printf("Введите номер контакта для редактирования (1-%d): ", count);
            int idx = input_number();
            
            if (idx < 1 || idx > count) { 
                printf("Неверный номер.\n"); 
                continue; 
            }
            
            Contact updated = contacts[idx - 1];
            char temp[100];
            
            printf("Новое имя (Enter - оставить [%s]): ", updated.first_name); 
            input_string(temp, 50);
            if (strlen(temp) > 0) strcpy(updated.first_name, temp);

            printf("Новая фамилия (Enter - оставить [%s]): ", updated.last_name); 
            input_string(temp, 50);
            if (strlen(temp) > 0) strcpy(updated.last_name, temp);

            printf("Новое место работы (Enter - оставить [%s]): ", updated.job); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.job, temp);

            printf("Новая должность (Enter - оставить [%s]): ", updated.position); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.position, temp);

            printf("Новый телефон (Enter - оставить [%s]): ", updated.phone); 
            input_string(temp, 20);
            if (strlen(temp) > 0) strcpy(updated.phone, temp);

            printf("Новый email (Enter - оставить [%s]): ", updated.email); 
            input_string(temp, 50);
            if (strlen(temp) > 0) strcpy(updated.email, temp);

            printf("Новые соцсети (Enter - оставить [%s]): ", updated.social); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.social, temp);

            printf("Новые мессенджеры (Enter - оставить [%s]): ", updated.messenger); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.messenger, temp);

            if (edit_contact(contacts, count, idx - 1, updated) == 0) {
                printf("Контакт успешно обновлен!\n");
            }
        }
        else if (choice == 4) {
            if (count == 0) { 
                printf("Книга пуста. Нечего удалять.\n"); 
                continue; 
            }
            printf("Введите номер контакта для удаления (1-%d): ", count);
            int idx = input_number();
            
            if (delete_contact(contacts, &count, idx - 1) == 0) {
                printf("Контакт успешно удален!\n");
            } else {
                printf("Неверный номер!\n");
            }
        }
        else {
            printf("Неизвестная команда. Попробуйте снова.\n");
        }
    }
    printf("Выход из программы.\n");
    return 0;
}
