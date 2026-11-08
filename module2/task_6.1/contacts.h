#ifndef CONTACTS_H
#define CONTACTS_H

typedef struct {
    char first_name[50];
    char last_name[50];
    char job[100];
    char position[100];
    char phone[20];
    char email[50];
    char social[100];
    char messenger[100];
} Contact;

typedef struct Node {
    Contact data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct {
    Node *head;
    int count;
} ContactList;

void init_list(ContactList *list);
void free_list(ContactList *list);

int add_contact(ContactList *list, Contact new_contact);
int delete_contact(ContactList *list, int index);
int edit_contact(ContactList *list, int index, Contact updated_contact);

void print_list_debug(ContactList *list);

#endif
