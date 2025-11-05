/*
Nomes:
  Caleb Martim -- 221017060
  Gabriel Henrique do Nascimento -- 221029140
  Guilherme da Rocha -- 221030007
  Hiago Sousa -- 221002049

Compilador: gcc 13.3.0

Sistema Operacional: LINUX 6.14.0-33-generic
*/


#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define true 1
#define false 0

const char *exit_s = "exit_scheduler";
const char *list_s = "list_scheduler";
const char *exec_s = "execute_process";
const char *user_s = "create_user_scheduler";

int string_to_int(char* s) {
  int len = strlen(s);
  int pot_10 = 1;
  int ret = 0;
  while(--len >= 0) {
    ret += pot_10 * (s[len] - '0');
    pot_10 *= 10;
  }

  return ret;
}


int main(){
  char command[100];

  while (true) {
    printf(">shell_sched: ");
    
    if(fgets(command, sizeof(command), stdin) == NULL) { // fgets le toda a linha
      fprintf(stderr, "Erro: falha ao ler o comando.\n");
      exit(EXIT_FAILURE);
    }
    command[strcspn(command,"\n")] = '\0'; // remove o '\n'

    if(strcmp(command, exit_s) == 0) {
      /*TODO - termina o escalonador (processo filho) e mostra o turnaround de cada processos 
      e os processos não finalizados*/

      break;
    }
    
    if(strcmp(command, list_s) == 0) {
      /*TODO - mostrar os processos nas filas e o processo em execucao*/

      continue;
    }

    char *token = strtok(command, " ");
    if(token != NULL) {
      if(strcmp(token, user_s) == 0) {
        int n = string_to_int(strtok(NULL, " ")); // numero de filas

        /*TODO - criar o processo filho para criar as filas e escalonar os processos*/

        continue;
      }

      if(strcmp(token, exec_s) == 0) {
        int pr = string_to_int(strtok(NULL, " ")); // prioridade

        /*TODO - mandar pro processo filho as infos do novo processo a ser escalonado*/

        continue;
      }

    }

    fprintf(stderr,"Erro: Comando não definido.\n");
    exit(EXIT_FAILURE); // exit(1) | return 1
  }

  /*TODO - frees necessarios*/

  return EXIT_SUCCESS; // return 0
}
