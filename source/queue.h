#ifndef QUEUE_H
#define QUEUE_H

#include "stdlib.h"
#include "process.h"

typedef struct Queue {
    int size;
    Process *front;
    Process *back;
} Queue;

Queue* new_queue(int p) {
    Queue* q = (Queue*) malloc(sizeof(Queue));

    if(q == NULL) return NULL;

    q->size = 0;
    q->front = q->back = NULL;

    return q;
}

int enqueue(Queue *q, Process* process) {
    if(q == NULL) return 1;

    if(q->size == 0) q->front = q->back = process;
    else {
        q->back->nxt = process;
        q->back = process;
    }

    q->size++;

    return 0;
}

int dequeue(Queue *q) {
    if(q->size == 0) return 1;

    q->front = q->front->nxt;
    q->size--;

    return 0;
}

void free_queue(Queue *q) {
    Process *process = q->front;
    while(process != NULL) {
        Process* nxt = process->nxt;
        free(process);
        process = nxt;
    }

    free(q);
    return;
}

int is_empty(Queue* q) {
    return (q->size == 0);
}

Process* front(Queue* q) {
    return q->front;
}

#endif