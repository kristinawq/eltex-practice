#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

int is_valid_name(const char *str) {
    if (strlen(str) == 0) return 0; 
    for (int i = 0; str[i] != '\0'; i++) {
        unsigned char c = (unsigned char)str[i];
        if (!isalpha(c) && c < 128) {
            return 0; 
        }
    }
    return 1;
}

int is_valid_phone(const char *str) {
    if (strlen(str) == 0) return 1; 
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (!isdigit(c) && c != ' ' && c != '+' && c != '-' && c != '(' && c != ')') {
            return 0;
        }
    }
    return 1;
}

int is_valid_email(const char *str) {
    if (strlen(str) == 0) return 1;
    const char *at = strchr(str, '@');
    if (!at) return 0;
    const char *dot = strchr(at, '.');
    if (!dot) return 0;
    return 1;
}

void print_contact(Contact c, int index) {
    printf("[%d] %s %s\n", index + 1, c.last_name, c.first_name);
    if (strlen(c.job) > 0) printf("Работа: %s\n", c.job);
    if (strlen(c.position) > 0) printf("Должность: %s\n", c.position);
    if (strlen(c.phone) > 0) printf("Телефон: %s\n", c.phone);
    if (strlen(c.email) > 0) printf("Email: %s\n", c.email);
    if (strlen(c.social) > 0) printf("Соцсети: %s\n", c.social);
    if (strlen(c.messenger) > 0) printf("Мессенджеры: %s\n", c.messenger);
    printf("\n");
}

void print_all_contacts(ContactList *list) {
    if (list->count == 0) {
        printf("Книга пуста.\n");
        return;
    }
    printf("\nВсего контактов: %d\n", list->count);
    Node *current = list->head;
    int i = 0;
    while (current != NULL) {
        print_contact(current->data, i);
        current = current->next;
        i++;
    }
}

int main() {
    ContactList list;
    init_list(&list);
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
            
            while (1) {
                printf("Введите имя: "); 
                input_string(new_c.first_name, 50);
                if (is_valid_name(new_c.first_name)) break;
                printf("Ошибка: Имя должно содержать только буквы\n");
            }

            while (1) {
                printf("Введите фамилию: "); 
                input_string(new_c.last_name, 50);
                if (is_valid_name(new_c.last_name)) break;
                printf("Ошибка: Фамилия должна содержать только буквы\n");
            }

            printf("Место работы (Enter - пропустить): "); input_string(new_c.job, 100);
            printf("Должность (Enter - пропустить): "); input_string(new_c.position, 100);
            
            while (1) {
                printf("Телефон (Enter - пропустить): "); 
                input_string(new_c.phone, 20);
                if (is_valid_phone(new_c.phone)) break;
                printf("Ошибка: Телефон может содержать только цифры, +, -, (, )\n");
            }

            while (1) {
                printf("Email (Enter - пропустить): "); 
                input_string(new_c.email, 50);
                if (is_valid_email(new_c.email)) break;
                printf("Ошибка: Email должен содержать символ '@'\n");
            }

            printf("Соцсети (Enter - пропустить): "); input_string(new_c.social, 100);
            printf("Мессенджеры (Enter - пропустить): "); input_string(new_c.messenger, 100);

            if (add_contact(&list, new_c) == 0) {
                printf("Контакт успешно добавлен\n");
            } else {
                printf("Ошибка памяти\n");
            }
        } 
        else if (choice == 2) {
	    print_list_debug(&list);
            print_all_contacts(&list);
        }
        else if (choice == 3) {
            if (list.count == 0) { 
                printf("Книга пуста.\n"); 
                continue; 
            }
            printf("Введите номер контакта для редактирования (1-%d): ", list.count);
            int idx = input_number();

            if (idx < 1 || idx > list.count) { 
                printf("Неверный номер.\n"); 
                continue; 
            }

            Node *curr = list.head;
            for (int i = 0; i < idx - 1; i++) curr = curr->next;
            Contact updated = curr->data;
            char temp[100];

            printf("Новое имя (Enter - оставить [%s]): ", updated.first_name); 
            input_string(temp, 50);
            if (strlen(temp) > 0) {
                if (is_valid_name(temp)) strcpy(updated.first_name, temp);
                else printf("Ошибка: неверный формат имени. Оставлено старое\n");
            }

            printf("Новая фамилия (Enter - оставить [%s]): ", updated.last_name); 
            input_string(temp, 50);
            if (strlen(temp) > 0) {
                if (is_valid_name(temp)) strcpy(updated.last_name, temp);
                else printf("Ошибка: неверный формат фамилии. Оставлено старое.\n");
            }

            printf("Новое место работы (Enter - оставить [%s]): ", updated.job); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.job, temp);

            printf("Новая должность (Enter - оставить [%s]): ", updated.position); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.position, temp);

            printf("Новый телефон (Enter - оставить [%s]): ", updated.phone); 
            input_string(temp, 20);
            if (strlen(temp) > 0) {
                if (is_valid_phone(temp)) strcpy(updated.phone, temp);
                else printf("Ошибка: неверный формат телефона. Оставлено старое.\n");
            }

            printf("Новый email (Enter - оставить [%s]): ", updated.email); 
            input_string(temp, 50);
            if (strlen(temp) > 0) {
                if (is_valid_email(temp)) strcpy(updated.email, temp);
                else printf("Ошибка: неверный формат email. Оставлено старое.\n");
            }

            printf("Новые соцсети (Enter - оставить [%s]): ", updated.social); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.social, temp);

            printf("Новые мессенджеры (Enter - оставить [%s]): ", updated.messenger); 
            input_string(temp, 100);
            if (strlen(temp) > 0) strcpy(updated.messenger, temp);

            if (edit_contact(&list, idx - 1, updated) == 0) {
                printf("Контакт успешно обновлен\n");
            }
        }
        else if (choice == 4) {
            if (list.count == 0) { 
                printf("Книга пуста\n"); 
                continue; 
            }
            printf("Введите номер контакта для удаления (1-%d): ", list.count);
            int idx = input_number();

            if (delete_contact(&list, idx - 1) == 0) {
                printf("Контакт успешно удален.\n");
            } else {
                printf("Неверный номер\n");
            }
        }
        else {
            printf("Неизвестная команда.\n");
        }
    }
    
    free_list(&list);
    return 0;
}
