#ifndef PROCESS_H
#define PROCESS_H

#include "stdlib.h"
#include "stdbool.h"
#include "time.h"

typedef struct Process {
    int pid;
    time_t time_init;
    int priority;
    long turnaround;
} Process;

Process* new_process(int pid, int priority) {
    Process* p = (Process*) malloc(sizeof(Process));
    if(p == NULL) return NULL;

    p->pid = pid;
    p->time_init = time(NULL);
    p->priority = priority;
    p->turnaround = 0;
    return p;
}

#endif