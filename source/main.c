/*
Nomes: 
  Caleb Martim de Oliveira -- 221017060
  Gabriel Henrique do Nascimento Neres -- 221029140
  Guilherme da Rocha Cunha -- 221030007
  Hiago Sousa Rocha -- 221002049

Compilador: gcc 13.3.0
Compilador de verdade: gcc 15.2.1

Sistema operacional: LINUX 6.14.0-33-generic
Sistema operacional de verdade: 6.17.8-arch1-1
LOONA - [+ +]
https://youtu.be/yymyRBvD79A
*/

// Bibliotecas para funcionamento 
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "unistd.h"

// Inclui funcionamento que guarda pid e turnaround de processsos
#include "process.h"
// Implementação de fila que será usada na implementação do Round-Robin
#include "queue.h"

// Header de funções e structs auxiliares:
#include "aux.h"

// Bibliotecas de mecanismos de comunicacao
#include "sys/types.h"
#include "sys/ipc.h"

// Biblioteca de fila de mensagem
#include "sys/msg.h"

// Biblioteca de sinais
#include "signal.h"

const char *exit_s = "exit_scheduler";
const char *list_s = "list_scheduler";
const char *exec_s = "execute_process";
const char *user_s = "create_user_scheduler";
const char *quit = "quit";

// Chave para a fila de mensagens
#define MSG_KEY 0x1233

int pid_sched;  // pid do processo responsável pelo escalonador
int n_filas;    // numero de filas round-robin
int msg_id;     // id da fila de mensagens

void exit_prog() {
  // frees necessarios 
  printf("Fechando interface...\n");

  msgctl(msg_id, IPC_RMID, NULL); // free na fila de mensagens
  kill(pid_sched, SIGINT); // mata o sched
}

int main(){
  signal(SIGSEGV, exit_prog);  // define de tratamento para segfault
  signal(SIGUSR1, exit_prog);  // define o tratamento de erros gerais (?)

  // String para armazenar os comandos recebidos pela interface
  char command[100];

  // Cria a fila de mensagens
  msg_id = msgget(MSG_KEY, IPC_CREAT | 0777);
  if(msg_id < 0) {
    fprintf(stderr, "Erro: falha ao criar a fila de mensagens.\n");
    exit(EXIT_FAILURE);
  }

  // Inicialização da struct mensagem
  mensagem mensagem_main;
  mensagem_main.pid = getpid();

  while (true) {
    printf(">shell_sched: "); fflush(stdout);

    if(fgets(command, sizeof(command), stdin) == NULL) { // fgets le toda a linha
      fprintf(stderr, "Erro: falha ao ler o comando.\n");
      kill(getpid(), SIGUSR1);   // rotina de saida
      exit(EXIT_FAILURE);
    }

    // remove o '\n'
    command[strcspn(command, "\n")] = '\0'; 

    // exit scheduler
    if(strcmp(command, exit_s) == 0) {
      kill(pid_sched, SIGINT);
      continue;
    };

    // list scheduler
    if(strcmp(command, list_s) == 0) {
      kill(pid_sched, SIGUSR1);        // pede as informacoes para o sched
      continue;
    }

    // Termina a execução da interface 
    if(strcmp(command, quit) == 0) break;

    // TODO: explicar o que está acontecendo aqui
    char *token = strtok(command, " ");
    if (token != NULL) {

      // Inicializa scheduler
      if (strcmp(token, user_s) == 0) {
        char* n = strtok(NULL, " ");  // numero de filas
        n_filas = atoi(n);            // casting pra int

        // salva o pid do sched
        pid_sched = fork();
        if (pid_sched == 0) {
          // Casting para char* das ids
          char arg_msg[20];
          sprintf(arg_msg, "%d", msg_id);

          // chama o executavel do "sched.c"
          execl("source/sched", "sched", n, arg_msg, NULL);

          // Entra aqui apenas se o comando acima não funcionou
          fprintf(stderr, "Erro: falha ao executar o comando 'execl'.\n");
          
          // frees necessarios
          kill(getpid(), SIGUSR1);   // rotina de saida
          exit(EXIT_FAILURE);
        }
        continue;
      }

      // Inicializa um novo processo no escalonador
      if(strcmp(token, exec_s) == 0) {
        char* pr = strtok(NULL, " "); // prioridade

        kill(pid_sched, SIGUSR2);   // avisa o sched q ha um novo processo
        strcpy(mensagem_main.msg, pr);  // copia a prioridade pro buffer de mensagem
        msgsnd(msg_id, &mensagem_main, sizeof(mensagem_main.msg), 0); // envia a prioridade pro sched

        continue;
      }
    }

    // Comando passado na interface não reconhecido
    fprintf(stderr,"Erro: Comando não definido.\n");

    // frees necessarios
    kill(getpid(), SIGUSR1);   // rotina de saida
    exit(EXIT_FAILURE); // exit(1) | return 1
  }

  kill(getpid(), SIGUSR1);   // rotina de saida
  return EXIT_SUCCESS; // return 0
}
