#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

Contact make_contact(const char *first, const char *last) {
    Contact c = {0};
    strncpy(c.first_name, first, sizeof(c.first_name) - 1);
    strncpy(c.last_name, last, sizeof(c.last_name) - 1);
    return c;
}

void input_name(const char *prompt, char *buffer) {
    while (1) {
        printf("%s", prompt);
        if (scanf("%49s", buffer) == 1) return;
        clear_input();
    }
}

int main(void) {
    AVLNode *root = NULL;
    int choice;

    while (1) {
        printf("\n");
        printf("1. Добавить контакт\n");
        printf("2. Показать дерево\n");
        printf("3. Обход: Прямой\n");
        printf("4. Обход: Симметричный\n");
        printf("5. Обход: Обратный\n");
        printf("6. Найти контакт\n");
        printf("7. Изменить контакт\n");
        printf("8. Удалить контакт\n");
        printf("0. Выход\n");
        printf("Выбор: ");

        if (scanf("%d", &choice) != 1) { clear_input(); continue; }
        clear_input();

        if (choice == 0) break;

        if (choice == 1) {
            Contact c = {0};
            input_name("Фамилия: ", c.last_name);
            input_name("Имя: ", c.first_name);
            root = insert_node(root, c);
            printf("Добавлено.\n");
        }
        else if (choice == 2) {
            if (root == NULL) printf("Дерево пусто.\n");
            else { printf("\nДерево:\n"); print_tree(root, 0); }
        }
        else if (choice == 3) { print_preorder(root); printf("\n"); }
        else if (choice == 4) { print_inorder(root); printf("\n"); }
        else if (choice == 5) { print_postorder(root); printf("\n"); }
        else if (choice == 6) {
            Contact c = {0};
            input_name("Фамилия: ", c.last_name);
            input_name("Имя: ", c.first_name);
            int steps = 0;
            if (find_node_debug(root, c, &steps)) printf("Найдено за %d шагов.\n", steps);
            else printf("Не найдено.\n");
        }
        else if (choice == 7) {
            Contact old_c = {0}, new_c = {0};
            input_name("Фамилия для изменения: ", old_c.last_name);
            input_name("Имя для изменения: ", old_c.first_name);

            int steps = 0;
            if (find_node_debug(root, old_c, &steps) == NULL) { printf("Не найдено.\n"); continue; }

            input_name("Новая фамилия: ", new_c.last_name);
            input_name("Новое имя: ", new_c.first_name);

            root = delete_node(root, old_c);
            root = insert_node(root, new_c);
            printf("Изменено и перебалансировано.\n");
        }
        else if (choice == 8) {
            Contact c = {0};
            input_name("Фамилия: ", c.last_name);
            input_name("Имя: ", c.first_name);
            root = delete_node(root, c);
            printf("Удалено и перебалансировано.\n");
        }
    }
    free_tree(root);
    return 0;
}
