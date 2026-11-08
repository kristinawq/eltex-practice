#include "contacts.h"

int add_contact(Contact contacts[], int *count, Contact new_contact) {
    if (*count >= MAX_CONTACTS) {
        return -1;
    }
    contacts[*count] = new_contact;
    (*count)++;
    return 0;
}

int delete_contact(Contact contacts[], int *count, int index) {
    if (index < 0 || index >= *count) {
        return -1;
    }
    for (int i = index; i < *count - 1; i++) {
        contacts[i] = contacts[i + 1];
    }
    (*count)--;
    return 0;
}

int edit_contact(Contact contacts[], int count, int index, Contact updated_contact) {
    if (index < 0 || index >= count) {
        return -1;
    }
    contacts[index] = updated_contact;
    return 0;
}
