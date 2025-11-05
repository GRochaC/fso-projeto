#ifndef PROCESS_H
#define PROCESS_H

#include "stdlib.h"

typedef struct Process {
    int pid;
    int t;
    int priority;
    int finished;
    int turnaround;
    struct Process *nxt;
} Process;

Process* new_process(int pid, int t, int pr) {
    Process* p = (Process*) malloc(sizeof(p));

    if(p == NULL) return NULL;

    p->pid = pid;
    p->t= t;
    p->nxt = NULL;
    p->priority = pr;
    p->finished = 0;
    p->turnaround = 0;
    return p;
}

#endif