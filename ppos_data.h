// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#define _XOPEN_SOURCE 600  // Não posso mexer na ppos.h, então adicionei esse define aqui para reconhecer na ppos_core.c

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include <valgrind/valgrind.h>

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  int vg_id ;   // Valgrind
  int static_prio;    // Prioridade estática da tarefa
  int dyn_prio;   // Prioridade dinâmica da tarefa (afetada pelo aging)
  int quantum_ticks;    // Contador de ticks do quantum da tarefa
  char task_type;   // Tipo da tarefa (de sistema ou de usuário)
  unsigned int execution_time;    // Tempo de vida total da tarefa
  unsigned int processor_time;    // Tempo de processamento acumulado
  unsigned int activations;   // Número de ativações da tarefa
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
