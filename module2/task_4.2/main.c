#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "queue.h"

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int input_priority(const char *prompt) {
    int p;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &p) != 1) {
            clear_input();
            printf("Ошибка: нужно ввести число\n");
            continue;
        }
        clear_input();
        if (p < 0 || p > 255) {
            printf("Ошибка: приоритет должен быть от 0 до 255\n");
            continue;
        }
        return p;
    }
}

void input_message(char *buffer, int size) {
    while (1) {
        printf("Текст сообщения: ");
        fgets(buffer, size, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) == 0) {
            printf("Ошибка: сообщение не может быть пустым\n");
            continue;
        }
        break;
    }
}

int main(void) {
    PriorityQueue q;
    init_queue(&q);
    int choice;

    while (1) {
        printf("\n");
        printf("1. Добавить сообщение\n");
        printf("2. Извлечь первое\n");
        printf("3. Извлечь с конкретным приоритетом\n");
        printf("4. Извлечь с приоритетом не ниже заданного\n");
        printf("5. Показать очередь\n");
        printf("6. Сгенерировать 5 случайных сообщений\n");
        printf("0. Выход\n");
        printf("Выбор: ");

        if (scanf("%d", &choice) != 1) { 
            clear_input(); 
            printf("Ошибка: введите число от 0 до 6.\n");
            continue; 
        }
        clear_input();

        if (choice == 0) break;

        if (choice == 1) {
            int p = input_priority("Приоритет: ");
            char msg[128];
            input_message(msg, sizeof(msg));
            
            if (push(&q, p, msg) == 0) {
                printf("Успешно добавлено в очередь.\n");
            } else {
                printf("Ошибка добавления.\n");
            }
        }
        else if (choice == 2) {
            if (q.head == NULL) {
                printf("Очередь пуста\n");
                continue;
            }
            Node *n = pop_first(&q);
            if (n) { 
                printf("Извлечено: [%d] %s\n", n->priority, n->message); 
                free(n); 
            }
        }
        else if (choice == 3) {
            if (q.head == NULL) {
                printf("Очередь пуста\n");
                continue;
            }
            int p = input_priority("Введите приоритет: ");
            
            Node *n = pop_by_priority(&q, p);
            if (n) { 
                printf("Извлечено: [%d] %s\n", n->priority, n->message); 
                free(n); 
            } else {
                printf("В очереди нет сообщений с приоритетом %d.\n", p);
            }
        }
        else if (choice == 4) {
            if (q.head == NULL) {
                printf("Очередь пуста\n");
                continue;
            }
            int th = input_priority("Введите порог приоритета: ");
            
            Node *n = pop_at_least(&q, th);
            if (n) { 
                printf("Извлечено: [%d] %s\n", n->priority, n->message); 
                free(n); 
            } else {
                printf("В очереди нет сообщений с приоритетом <= %d.\n", th);
            }
        }
        else if (choice == 5) {
            printf("Текущая очередь (%d элементов):\n", q.count);
            print_queue(&q);
        }
        else if (choice == 6) {
            printf("Генерация 5 случайных сообщений\n");
            for(int i = 0; i < 5; i++) {
                int p = rand() % 256;
                char buf[64];
                snprintf(buf, sizeof(buf), "Сообщение %d", i + 1);
                push(&q, p, buf);
            }
        }
        else {
            printf("Неизвестная команда. Введите число от 0 до 6.\n");
        }
    }
    free_queue(&q);
    printf("Выход.\n");
    return 0;
}
