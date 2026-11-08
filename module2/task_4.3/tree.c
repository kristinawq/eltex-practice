#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int get_height(AVLNode *node) { return node ? node->height : 0; }
int max(int a, int b) { return (a > b) ? a : b; }

int compare_contacts(Contact a, Contact b) {
    int cmp_last = strcmp(a.last_name, b.last_name);
    if (cmp_last != 0) return cmp_last;
    return strcmp(a.first_name, b.first_name);
}

AVLNode* rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

AVLNode* rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    return y;
}

int get_balance(AVLNode *node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

AVLNode* insert_node(AVLNode *node, Contact c) {
    if (node == NULL) {
        AVLNode *new_node = (AVLNode*)malloc(sizeof(AVLNode));
        new_node->data = c;
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    int cmp = compare_contacts(c, node->data);
    if (cmp < 0) node->left = insert_node(node->left, c);
    else if (cmp > 0) node->right = insert_node(node->right, c);
    else return node;

    node->height = 1 + max(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && compare_contacts(c, node->left->data) < 0) return rotate_right(node);
    if (balance < -1 && compare_contacts(c, node->right->data) > 0) return rotate_left(node);
    if (balance > 1 && compare_contacts(c, node->left->data) > 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && compare_contacts(c, node->right->data) < 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

AVLNode* find_max(AVLNode *node) {
    AVLNode *current = node;
    while (current && current->right != NULL) current = current->right;
    return current;
}

AVLNode* delete_node(AVLNode *root, Contact c) {
    if (root == NULL) return root;

    int cmp = compare_contacts(c, root->data);
    if (cmp < 0) root->left = delete_node(root->left, c);
    else if (cmp > 0) root->right = delete_node(root->right, c);
    else {
        if ((root->left == NULL) || (root->right == NULL)) {
            AVLNode *temp = root->left ? root->left : root->right;
            if (temp == NULL) { temp = root; root = NULL; }
            else *root = *temp;
            free(temp);
        } else {
            AVLNode *temp = find_max(root->left);
            root->data = temp->data;
            root->left = delete_node(root->left, temp->data);
        }
    }

    if (root == NULL) return root;

    root->height = 1 + max(get_height(root->left), get_height(root->right));
    int balance = get_balance(root);

    if (balance > 1 && get_balance(root->left) >= 0) return rotate_right(root);
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance < -1 && get_balance(root->right) <= 0) return rotate_left(root);
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }
    return root;
}

AVLNode* find_node_debug(AVLNode *node, Contact c, int *steps) {
    if (node == NULL) return NULL;
    (*steps)++;
    printf("  Шаг %d: [%s %s] -> ", *steps, node->data.last_name, node->data.first_name);
    int cmp = compare_contacts(c, node->data);
    if (cmp == 0) { printf("СОВПАДЕНИЕ!\n"); return node; }
    else if (cmp < 0) { printf("ВЛЕВО\n"); return find_node_debug(node->left, c, steps); }
    else { printf("ВПРАВО\n"); return find_node_debug(node->right, c, steps); }
}

void print_preorder(AVLNode *root) {
    if (root != NULL) { printf("[%s %s] ", root->data.last_name, root->data.first_name); print_preorder(root->left); print_preorder(root->right); }
}
void print_inorder(AVLNode *root) {
    if (root != NULL) { print_inorder(root->left); printf("[%s %s] ", root->data.last_name, root->data.first_name); print_inorder(root->right); }
}
void print_postorder(AVLNode *root) {
    if (root != NULL) { print_postorder(root->left); print_postorder(root->right); printf("[%s %s] ", root->data.last_name, root->data.first_name); }
}

void print_tree(AVLNode *root, int space) {
    if (root == NULL) return;
    space += 7;
    print_tree(root->right, space);
    printf("\n");
    for (int i = 5; i < space; i++) printf(" ");
    printf("%s %s\n", root->data.last_name, root->data.first_name);
    print_tree(root->left, space);
}

void free_tree(AVLNode *root) {
    if (root != NULL) { free_tree(root->left); free_tree(root->right); free(root); }
}
