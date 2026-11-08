#ifndef TREE_H
#define TREE_H

typedef struct Contact {
    char first_name[50];
    char last_name[50];
} Contact;

typedef struct AVLNode {
    Contact data;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

AVLNode* insert_node(AVLNode *node, Contact c);
AVLNode* find_node_debug(AVLNode *node, Contact c, int *steps);
AVLNode* delete_node(AVLNode *root, Contact c);
void print_tree(AVLNode *root, int space);
void free_tree(AVLNode *root);
int get_height(AVLNode *node);

void print_preorder(AVLNode *root);
void print_inorder(AVLNode *root);
void print_postorder(AVLNode *root);

#endif
