/*
Nomes:
  Caleb Martim de Oliveira -- 221017060
  Gabriel Henrique do Nascimento Neres -- 221029140
  Guilherme da Rocha Cunha -- 221030007
  Hiago Sousa Rocha -- 221002049

Compilador: 
gcc 13.3.0

Sistema operacional: 
Ubuntu 24.04.3 LTS

Kernel: 
linux 6.14.0-33-generic

COMPILAÇÃO:
gcc -o bin/sched source/sched.c | gcc -o shell_sched source/main.c | gcc -o bin/proc_exec source/proc_exec.c

EXECUÇÃO: 
./shell_sched

LOONA - [+ +]
https://youtu.be/yymyRBvD79A

*/

// Bibliotecas para funcionamento
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

// Header de funções e structs auxiliares:
#include "aux.h"

// Bibliotecas de mecanismos de comunicacao
#include "sys/ipc.h"

// Biblioteca de fila de mensagem
#include "sys/msg.h"

// Biblioteca de sinais
#include "signal.h"

const char *exit_s = "exit_scheduler";
const char *list_s = "list_scheduler";
const char *exec_s = "execute_process";
const char *user_s = "create_user_scheduler";
const char *user_s2 = "user_scheduler";
const char *quit_sh = "quit";
const char *exit_sh = "exit";

// Chave para a fila de mensagens
#define MSG_KEY 0x1233

int pid_sched; // pid do processo responsável pelo escalonador
int n_filas;   // numero de filas round-robin
int msg_id;    // id da fila de mensagens

// Variável booleana que nos diz se há algum escalonador rodando
bool sched_running = false;

void exit_prog() {
  printf("Fechando interface...\n");

  msgctl(msg_id, IPC_RMID, NULL); // free na fila de mensagens
  kill(pid_sched, SIGINT);        // finaliza o processo do escalonador
}

int main() {
  signal(SIGSEGV, (void *)exit_prog); // define o tratamento de segfault
  signal(SIGUSR1, (void *)exit_prog); // define o tratamento de finalização do programa

  // String usada para armazenar os comandos recebidos pela interface
  char command[100];

  // Cria a fila de mensagens
  msg_id = msgget(MSG_KEY, IPC_CREAT | 0777);
  if (msg_id < 0) {
    fprintf(stderr, "Erro: falha ao criar a fila de mensagens.\n");
    exit(EXIT_FAILURE);
  }

  // Inicialização da struct mensagem para a fila de mensagens
  mensagem mensagem_main;
  mensagem_main.pid = getpid();

  // Loop principal da nossa interface
  while (true) {
    printf(">shell_sched: ");
    fflush(stdout);

    if (fgets(command, sizeof(command), stdin) == NULL) { // fgets le toda a linha
      fprintf(stderr, "Erro: falha ao ler o comando.\n");
      kill(getpid(), SIGUSR1); // rotina de saida
      exit(EXIT_FAILURE);
      continue;
    }

    // remove o '\n'
    command[strcspn(command, "\n")] = '\0';

    // exit scheduler
    if (strcmp(command, exit_s) == 0) {
      if (!sched_running) {
        fprintf(stderr, "Erro: escalonador não inicializado.\n");
        continue;
      }
      kill(pid_sched, SIGINT);
      sched_running = false;
      continue;
    };

    // list scheduler
    if (strcmp(command, list_s) == 0) {
      if (!sched_running) {
        fprintf(stderr, "Erro: escalonador não inicializado.\n");
        continue;
      }
      kill(pid_sched, SIGUSR1); // pede as informacoes para o sched
      continue;
    }

    // Shell recebeu um comando vazio
    if (strcmp(command, "") == 0)
      continue;

    // Termina a execução da interface
    if ((strcmp(command, quit_sh)) == 0 || (strcmp(command, exit_sh) == 0))
      break;

    // Captura a string que vem antes do cwractére espaço, se houver, do comando
    char *token = strtok(command, " ");

    if (token != NULL) {

      // Comando para inicializar o escalonador
      if ((strcmp(token, user_s) == 0) || (strcmp(token, user_s2) == 0)) {

        if (sched_running) {
          fprintf(stderr, "Erro: escalonador já existente.\n");
          continue;
        }

        char *n = strtok(NULL, " "); // numero de filas

        // Número de filas não passado no comando
        if (n == NULL) {
          fprintf(stderr, "Erro: número de filas inválido.\n");
          continue;
        }

        n_filas = atoi(n); // casting pra int

        if (n_filas < 1) {
          fprintf(stderr, "Erro: número de filas inválido.\n");
          continue;
        }

        // salva o pid do processo escalonador
        pid_sched = fork();

        if (pid_sched == 0) {
          // Casting para char* o id da fila de mensagem
          char arg_msg[20];
          sprintf(arg_msg, "%d", msg_id);

          // chama o executavel do programa definido em source/sched.c
          execl("bin/sched", "sched", n, arg_msg, NULL);

          // Entra aqui apenas se o comando acima não funcionou
          fprintf(stderr, "Erro: falha ao executar o comando 'execl'.\n");

          kill(getpid(), SIGUSR1); // rotina de saida
          exit(EXIT_FAILURE);
        }

        sched_running = true;
        continue;
      }

      // Comando para inicializar um novo processo no escalonador
      if (strcmp(token, exec_s) == 0) {
        char *pr = strtok(NULL, " "); // prioridade do processo

        // Prioridade não passada
        if (pr == NULL) {
          fprintf(stderr, "Erro: prioridade inválida.\n");
          continue;
        }

        if (!sched_running) {
          fprintf(stderr, "Erro: escalonador não inicializado.\n");
          continue;
        }

        if (atoi(pr) > n_filas || atoi(pr) < 1) {
          fprintf(stderr, "Erro: prioridade inválida.\n");
          continue;
        }

        // avisa o escalonador que há um novo processo
        kill(pid_sched, SIGUSR2); 
        // copia a prioridade do processo para o buffer de mensagem
        strcpy(mensagem_main.msg, pr); 
        // envia a prioridade do processo para o escalonador
        msgsnd(msg_id, &mensagem_main, sizeof(mensagem_main.msg), 0); 

        continue;
      }
    }

    // Comando passado na interface não reconhecido
    fprintf(stderr, "Erro: Comando não definido.\n");
  }

  kill(getpid(), SIGUSR1); // rotina de saida
  return EXIT_SUCCESS;
}
