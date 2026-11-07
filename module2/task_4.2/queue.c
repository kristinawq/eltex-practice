#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_queue(PriorityQueue *q) {
    q->head = NULL;
    q->count = 0;
}

void free_queue(PriorityQueue *q) {
    Node *current = q->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    q->head = NULL;
    q->count = 0;
}

int push(PriorityQueue *q, int priority, const char *msg) {
    if (priority < 0 || priority > 255) return -1;

    Node *new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return -1;

    new_node->priority = priority;
    strncpy(new_node->message, msg, sizeof(new_node->message) - 1);
    new_node->message[sizeof(new_node->message) - 1] = '\0';

    if (q->head == NULL || priority < q->head->priority) {
        new_node->next = q->head;
        q->head = new_node;
    } else {
        Node *current = q->head;
        while (current->next != NULL && current->next->priority <= priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
    q->count++;
    return 0;
}

Node* pop_first(PriorityQueue *q) {
    if (q->head == NULL) return NULL;
    Node *item = q->head;
    q->head = q->head->next;
    q->count--;
    return item;
}

Node* pop_by_priority(PriorityQueue *q, int priority) {
    if (q->head == NULL) return NULL;

    Node *current = q->head;
    Node *prev = NULL;

    while (current != NULL) {
        if (current->priority == priority) {
            if (prev == NULL) q->head = current->next;
            else prev->next = current->next;
            q->count--;
            return current;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

Node* pop_at_least(PriorityQueue *q, int threshold) {
    if (q->head == NULL) return NULL;

    Node *current = q->head;
    Node *best_match = NULL;
    Node *best_prev = NULL;
    Node *prev = NULL;

    while (current != NULL) {
        if (current->priority <= threshold) {
            if (best_match == NULL || current->priority > best_match->priority) {
                best_match = current;
                best_prev = prev;
            }
        } else if (current->priority > threshold) {
            break; 
        }
        prev = current;
        current = current->next;
    }

    if (best_match != NULL) {
        if (best_prev == NULL) q->head = best_match->next;
        else best_prev->next = best_match->next;
        q->count--;
        return best_match;
    }

    return NULL;
}

void print_queue(const PriorityQueue *q) {
    if (q->head == NULL) {
        printf("  [Очередь пуста]\n");
        return;
    }
    Node *current = q->head;
    while (current != NULL) {
        printf("  [%3d] %s\n", current->priority, current->message);
        current = current->next;
    }
}
