#ifndef AUX_H
#define AUX_H

// struct responsavel por armazenar as mensagens enviadas e recebidas pelos processos
typedef struct mensagem {
  long pid;
  char msg[100];
} mensagem;

#endif