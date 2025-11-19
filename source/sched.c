#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "unistd.h"
#include "sys/wait.h"

#include "process.h"
#include "queue.h"

// Header de funções auxiliares:
#include "aux.h"

// Bibliotecas de mecanismos de comunicacao
#include "sys/types.h"
#include "sys/ipc.h"

// Biblioteca de fila de mensagem
#include "sys/msg.h"

// Biblioteca de sinais
#include "signal.h"

bool voltar_para_o_inicio = false;  

int n;                              // numero de filas round-robins

Queue** round_robins = NULL;
Queue* finished_processes = NULL;

// declaracao da struct mensagem
mensagem mensagem_sched;

Process* processo_atual;
Process* processo_default;

int msg_id;     // id da fila de mensagens

// exit_scheduler
void exit_sched() {
    if (!is_empty(finished_processes)) {
        printf("\nProcessos finalizados:\n");
        Node *atual = finished_processes->front;

        while (atual != NULL) {
            Process *proc = atual->proc;
            printf("\tProcesso: %d, prioridade: %d, turnaround: %lds\n", proc->pid, proc->priority, proc->turnaround);
            atual = atual->nxt;
        }
    }

    // Processo atual:
    if(processo_atual->pid != 2147483647) printf("Processo atual: %d, prioridade: %d\n", processo_atual->pid, processo_atual->priority);
    
    printf("Processos não finalizados:\n");
    for (int pr = 0; pr < n; ++pr) {
        if (!is_empty(round_robins[pr])) {
            printf("\tFila de prioridade %d\n", pr+1);
            Node *atual = round_robins[pr]->front;

            while (atual != NULL) {
                Process *proc = atual->proc;
                printf("\t\tProcesso: %d\n", proc->pid);
                atual = atual->nxt;
            }
        }
    }

    // frees necessarios
    for(int i = 0; i < n; i++) free_queue(round_robins[i]);
    free_queue(finished_processes);

    free(round_robins);

    free(processo_atual);

    exit(EXIT_SUCCESS);
}

// list_scheduler
void info_sched() {
    // Processo atual:
    if(processo_atual->pid != 2147483647) printf("\nProcesso atual: %d, prioridade: %d\n", processo_atual->pid, processo_atual->priority);

    // Processos nas filas:
    for (int pr = 0; pr < n; ++pr) {
        if (!is_empty(round_robins[pr])) {
            printf("Fila de prioridade %d\n", pr+1);
            Node *atual = round_robins[pr]->front;

            while (atual != NULL) {
                Process *proc = atual->proc;
                printf("\tProcesso: %d\n", proc->pid);
                atual = atual->nxt;
            }
        }
    }
}

// execute_process
void add_proc() {
    strcpy(mensagem_sched.msg, "");  // limpa o buffer de mensagem
    msgrcv(msg_id, &mensagem_sched, sizeof(mensagem_sched.msg), 0, 0);    // recebe a prioridade da main
    
    int pid_new_process = fork();
    if(pid_new_process == 0) {
        kill(getpid(), SIGSTOP);     // espera receber SIGCONT

        execl("source/proc_exec", "proc_exec", (char *) 0);

        fprintf(stderr, "Erro: falha ao executar o comando 'execl'.\n");
    } else {
        Process* proc = new_process(pid_new_process, atoi(mensagem_sched.msg));
        push(round_robins[proc->priority-1], proc);
        voltar_para_o_inicio = true;
    }
}

int main(int argc, char* argv[]) {
    mensagem_sched.pid = getpid();

    n = atoi(argv[1]);                          // número de filas round-robins
    msg_id = atoi(argv[2]);                     // id da fila de mensagem

    round_robins = (Queue**) malloc(sizeof(Queue*) * n); // cria os arrays de filas rr
    processo_default = new_process(2147483647,-1);
    processo_atual = processo_default;

    signal(SIGINT, exit_sched);                // rotina de saida
    signal(SIGUSR1, info_sched);                // rotina de listar os processos
    signal(SIGUSR2, add_proc);                  // rotina de adicionar novo processo

    for (int i = 0; i < n; i++) round_robins[i] = new_queue();
    finished_processes = new_queue();

    while(true) {
        for(int pr = 0; pr < n; pr++) {
            if(voltar_para_o_inicio) {
                pr = -1;    // reseta o for
                voltar_para_o_inicio = false;
                continue;
            }

            // executamos todos os processos de prioridade == pr
            if(!is_empty(round_robins[pr])) {
                processo_atual = pop(round_robins[pr]); 
                // printf("Processando %d\n", processo_atual->pid);

                kill(processo_atual->pid, SIGCONT); // Acorda o processo_atual 

                // quantum 4s, signal proof
                int falta_dormir = 4;
                do {
                    falta_dormir = sleep(falta_dormir);
                } while(falta_dormir);

                int s;
                int status = waitpid(processo_atual->pid, &s, WNOHANG);

                if(status == processo_atual->pid) {
                    // printf("Processo %d finalizado\n", processo_atual->pid);
                    processo_atual->turnaround = time(NULL) - processo_atual->time_init;
                    push(finished_processes, processo_atual);   // processo terminou

                    processo_atual = processo_default;
                } else {
                    kill(processo_atual->pid, SIGSTOP); // para o processo
                    push(round_robins[pr], processo_atual); // coloca no final da fila
                }

                if(!is_empty(round_robins[pr])) pr--;   // permanece na prioridade pr na próxima iteração
            }
        }
    }
}