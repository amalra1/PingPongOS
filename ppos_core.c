/*

Nome: Pedro Amaral Chapelin

GRR: 20206145

*/
#include <stdio.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue/queue.h"

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

#define TASK_PRONTA 0
#define TASK_TERMINADA 1
#define TASK_SUSPENSA 2

#define FATOR_ENVELHECIMENTO -1
#define PRIOR_MIN -20
#define PRIOR_MAX 20 

task_t taskMain;         // Descritor da tarefa principal
task_t *taskAtual;       // Ponteiro para a tarefa atual
task_t taskDispatcher;
queue_t *readyQueue = NULL;
int userTasks = 0;       // Quantidade de tasks
int next_id = 1;         // Contador para gerar IDs de novas tarefas (main é a 0)

void dispatcher (void *arg);
task_t* scheduler ();

// Por prioridade dinâmica
task_t* scheduler () {
    if (!readyQueue)
        return NULL;

    task_t *current = (task_t*) readyQueue;
    task_t *task_maior_prior = current;

    // Encontra a tarefa com a maior prioridade (menor valor de dyn_prio pelo aging definido)
    for (int i = 0; i < queue_size((queue_t*) readyQueue); i++) {
        if (current->dyn_prio < task_maior_prior->dyn_prio)
            task_maior_prior = current;

        current = current->next;
    }

    // Faz o aging em todas as outras tarefas
    current = (task_t*) readyQueue;
    for (int i = 0; i < queue_size((queue_t*) readyQueue); i++) {
        if (current != task_maior_prior) {
            current->dyn_prio += FATOR_ENVELHECIMENTO;
            if (current->dyn_prio < -20)
                current->dyn_prio = -20;
        }
        current = current->next;
    }

    // Reseta a prioridade dinâmica da tarefa escolhida para sua prioridade estática
    task_maior_prior->dyn_prio = task_maior_prior->static_prio;

    // Remove a tarefa escolhida da fila e a retorna
    queue_remove(&readyQueue, (queue_t*) task_maior_prior);
    return task_maior_prior;
}

void dispatcher (void *arg) {
    task_t *nextTask;

    while (userTasks > 0) {
        nextTask = scheduler();
        if (nextTask) {
            #ifdef DEBUG
            printf("dispatcher: ativando tarefa %d\n", nextTask->id);
            #endif
            task_switch(nextTask);

            switch(nextTask->status) {
                case TASK_TERMINADA:
                    #ifdef DEBUG
                        printf("dispatcher: tarefa %d terminada, liberando sua pilha.\n", nextTask->id);
                    #endif

                    VALGRIND_STACK_DEREGISTER(nextTask->vg_id);
                    free(nextTask->context.uc_stack.ss_sp);
                    break;
                case TASK_PRONTA:
                    break;
                default:
                    break;
            }
        }
    }
    task_exit(0);
}

void ppos_init () {

    setvbuf(stdout, 0, _IONBF, 0);

    // Inicia o id da main como 0 e aponta a atual para a da main
    taskMain.id = 0;
    taskAtual = &taskMain;

    // Para ter um ponto de retorno para a da main
    getcontext(&taskMain.context); 

    // Cria o dispatcher
    task_init(&taskDispatcher, dispatcher, NULL);
}

int task_init (task_t *task, void (*start_routine)(void *),  void *arg) {
    
    char *stack;

    if (!task)
      return -1;

    getcontext(&task->context);

    // Aloca espaço para a pilha da task
    stack = malloc (STACKSIZE);
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = &taskDispatcher.context;  // Pra voltar pro dispatcher
        task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);
    }
    else
    {
        perror ("Erro na criação da pilha: ");
        exit (1);
    }
    
    // Aponta a função start_routine para o contexto da task passada, passando os argumentos
    makecontext(&task->context, (void (*)(void))start_routine, 1, arg);

    task->id = next_id++;
    task->status = TASK_PRONTA;

    task->static_prio = 0;
    task->dyn_prio = 0;

    if (task != &taskDispatcher) {
        queue_append(&readyQueue, (queue_t*)task);
        userTasks++;
    }

    #ifdef DEBUG
        printf("task_init: iniciada tarefa %d\n", task->id);
    #endif

    return task->id;
}

int task_switch (task_t *task) {

    // Caso de por algum motivo a tarefa ser nula
    if (!task)
        return -1;

    task_t *taskAnterior = taskAtual;
    taskAtual = task;

    #ifdef DEBUG
        printf("task_switch: trocando contexto %d -> %d\n", taskAnterior->id, taskAtual->id);
    #endif

    // Salva o contexto da tarefa anterior e carrega o da nova tarefa
    swapcontext(&taskAnterior->context, &taskAtual->context);

    return 0;
}

void task_yield () {
    // Coloca no fim da fila de prontas
    taskAtual->status = TASK_PRONTA;
    queue_append(&readyQueue, (queue_t*)taskAtual);

    // Devolve o controle pro dispatcher
    task_switch(&taskDispatcher);
}

void task_exit (int exit_code) {

    #ifdef DEBUG
        printf("task_exit: tarefa %d sendo encerrada\n", task_id());
    #endif

    if (taskAtual == &taskDispatcher) {
        task_switch(&taskMain);
    }
    else if (taskAtual == &taskMain) {
        task_switch(&taskDispatcher);
    }
    else
    {
        userTasks--;
        taskAtual->status = TASK_TERMINADA;
        task_switch(&taskDispatcher);
    }
}

int task_id () {
    return taskAtual->id;
}

int task_getprio (task_t *task) {

    if (!task)
        return taskAtual->static_prio;
    return task->static_prio;
}

void task_setprio (task_t *task, int prio) {

    if (!task)
        task = taskAtual;

    // Mantém as prioridades dentro do intervalo caso ultrapassem
    if (prio < PRIOR_MIN) 
        prio = PRIOR_MIN;

    else if (prio > PRIOR_MAX) 
        prio = PRIOR_MAX;

    task->static_prio = prio;
    task->dyn_prio = prio;
}