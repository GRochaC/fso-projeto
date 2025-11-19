#ifndef QUEUE_H
#define QUEUE_H

#include "stdlib.h"
#include "process.h"

#include "signal.h"

typedef struct Node {
    Process *proc;
    struct Node *nxt;
} Node;

typedef struct Queue {
    int size;
    Node *front;
    Node *back;
} Queue;

Queue* new_queue() {
    Queue* q = (Queue*) malloc(sizeof(Queue));

    if (q == NULL) return NULL;

    q->size = 0;
    q->front = q->back = NULL;

    return q;
}

int push(Queue *q, Process* process) {
    if (q == NULL) return 1;

    Node* no = (Node*) malloc(sizeof(Node));
    if (no == NULL) return 1;
    no->proc = process;
    no->nxt = NULL;

    if (q->size == 0) q->front = q->back = no;
    else {
        q->back->nxt = no;
        q->back = no;
    }

    q->size++;

    return 0;
}

// Retira o primeiro elemento da fila q
Process* pop(Queue *q) {
    if (q->size == 0) return NULL;

    Node* no = q->front;
    q->front = q->front->nxt;
    q->size--;

    Process* proc = no->proc;
    free(no);

    return proc;
}

void free_queue(Queue *q) {
    if(q == NULL) return;

    Node *node = q->front;
    while (node != NULL) {
        Node* nxt = node->nxt;
        Process* p = pop(q);
        kill(p->pid, SIGKILL);
        free(p);
        node = nxt;
    }

    free(q);
    return;
}

bool is_empty(Queue* q) {
    return (q->size == 0);
}

#endif