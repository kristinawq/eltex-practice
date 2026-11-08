#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    int priority;
    char message[128];
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    int count;
} PriorityQueue;

void init_queue(PriorityQueue *q);
void free_queue(PriorityQueue *q);
int push(PriorityQueue *q, int priority, const char *msg);
Node* pop_first(PriorityQueue *q);
Node* pop_by_priority(PriorityQueue *q, int priority);
Node* pop_at_least(PriorityQueue *q, int threshold);
void print_queue(const PriorityQueue *q);

#endif
