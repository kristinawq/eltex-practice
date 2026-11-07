#ifndef CONTACTS_H
#define CONTACTS_H

#define MAX_CONTACTS 100

typedef struct {
    char first_name[50];
    char last_name[50];
    char job[100];
    char position[100];
    char phone[20];
    char email[100];
    char social[100];
    char messenger[100];
} Contact;

int add_contact(Contact contacts[], int *count, Contact new_contact);
int delete_contact(Contact contacts[], int *count, int index);
int edit_contact(Contact contacts[], int count, int index, Contact updated_contact);

#endif
