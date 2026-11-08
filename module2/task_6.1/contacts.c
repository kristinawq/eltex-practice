#include "contacts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_list(ContactList *list) {
    list->head = NULL;
    list->count = 0;
}

void free_list(ContactList *list) {
    Node *current = list->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
    list->count = 0;
}

int add_contact(ContactList *list, Contact new_contact) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return -1;

    new_node->data = new_contact;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (list->head == NULL || 
        strcmp(new_contact.last_name, list->head->data.last_name) < 0 ||
        (strcmp(new_contact.last_name, list->head->data.last_name) == 0 && 
         strcmp(new_contact.first_name, list->head->data.first_name) < 0)) {
        
        new_node->next = list->head;
        if (list->head != NULL) list->head->prev = new_node;
        list->head = new_node;
    } else {
        Node *current = list->head;
        while (current->next != NULL) {
            int cmp_last = strcmp(new_contact.last_name, current->next->data.last_name);
            int cmp_first = strcmp(new_contact.first_name, current->next->data.first_name);
            
            if (cmp_last < 0 || (cmp_last == 0 && cmp_first < 0)) {
                break; 
            }
            current = current->next;
        }
        
        new_node->next = current->next;
        if (current->next != NULL) current->next->prev = new_node;
        current->next = new_node;
        new_node->prev = current;
    }
    list->count++;
    return 0;
}

int delete_contact(ContactList *list, int index) {
    if (index < 0 || index >= list->count) return -1;

    Node *current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }

    if (current->prev != NULL) current->prev->next = current->next;
    else list->head = current->next; 

    if (current->next != NULL) current->next->prev = current->prev;

    free(current);
    list->count--;
    return 0;
}

int edit_contact(ContactList *list, int index, Contact updated_contact) {
    if (index < 0 || index >= list->count) return -1;

    Node *current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    delete_contact(list, index); 
    
    return add_contact(list, updated_contact);    
}

void print_list_debug(ContactList *list) {
    if (list->head == NULL) {
        printf("список пуст\n");
        return;
    }
    printf("head -> ");
    Node *current = list->head;
    while (current != NULL) {
        printf("[%s %s]", current->data.first_name, current->data.last_name);
        
        if (current->next != NULL) {
            printf(" <-> ");
        } else {
            printf(" -> NULL\n");
        }
        current = current->next;
    }
}
