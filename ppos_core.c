/*

Nome: Pedro Amaral Chapelin

GRR: 20206145

*/
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "queue/queue.h"

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

#define TASK_PRONTA 0
#define TASK_TERMINADA 1
#define TASK_SUSPENSA 2

#define FATOR_ENVELHECIMENTO -1
#define PRIOR_MIN -20
#define PRIOR_MAX 20 

#define QUANTUM_DEFAULT 10
#define SYSTEM_TASK 0
#define USER_TASK 1

task_t taskMain;         // Descritor da tarefa principal
task_t *taskAtual;       // Ponteiro para a tarefa atual
task_t taskDispatcher;
queue_t *readyQueue = NULL;
int userTasks = 0;       // Quantidade de tasks
int next_id = 1;         // Contador para gerar IDs de novas tarefas (main é a 0)

static unsigned int system_clock = 0;

struct sigaction action;
struct itimerval timer;

void dispatcher (void *arg);
task_t* scheduler ();
void task_awake (task_t * task, task_t **queue);

void tick_handler(int signum) {

    system_clock++;

    if (taskAtual)
        taskAtual->processor_time++;

    if (taskAtual && taskAtual->task_type == USER_TASK) {
        taskAtual->quantum_ticks--;

        if (taskAtual->quantum_ticks <= 0)
            task_yield();
    }
}

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

    while (userTasks > 0 || readyQueue != NULL) {
        nextTask = scheduler();
        if (nextTask) {
            nextTask->quantum_ticks = QUANTUM_DEFAULT;
            task_switch(nextTask);

            switch(nextTask->status) {
                case TASK_TERMINADA:
                    VALGRIND_STACK_DEREGISTER(nextTask->vg_id);
                    free(nextTask->context.uc_stack.ss_sp);
                    break;
                case TASK_PRONTA:
                case TASK_SUSPENSA:
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
    taskMain.task_type = USER_TASK;
    taskMain.quantum_ticks = QUANTUM_DEFAULT;

    // Para ter um ponto de retorno para a da main
    getcontext(&taskMain.context); 

    // Inicializa a contabilização da main, já começa ativa
    taskMain.execution_time = systime();
    taskMain.processor_time = 0;
    taskMain.activations = 1;
    taskMain.waiting_queue = NULL;

    // Registra a ação para o sinal de timer SIGALRM
    action.sa_handler = tick_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0) {
        perror("Erro em sigaction");
        exit(1);
    }

    // Configura o timer para chamar a cada 1ms
    timer.it_value.tv_usec = 1000;      // Primeiro chamada
    timer.it_value.tv_sec  = 0;
    timer.it_interval.tv_usec = 1000;       // Chamadas seguintes
    timer.it_interval.tv_sec  = 0;

    if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
        perror("Erro em setitimer");
        exit(1);
    }

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

    if (task != &taskMain)
        task->id = next_id++;
    task->status = TASK_PRONTA;
    task->static_prio = 0;
    task->dyn_prio = 0;

    // Contabilização dos tempos da task, diferente da main começa com 0 ativações
    task->execution_time = systime(); // Marca o tempo de criação
    task->processor_time = 0;
    task->activations = 0;

    task->waiting_queue = NULL;
    task->exit_code = 0;

    // Define o tipo da tarefa
    if (task == &taskDispatcher)
        task->task_type = SYSTEM_TASK;
    else if (task != &taskMain)
        task->task_type = USER_TASK;

    if (task != &taskDispatcher && task != &taskMain) {
        queue_append(&readyQueue, (queue_t*)task);
        userTasks++;
    }

    return task->id;
}

int task_switch (task_t *task) {

    // Caso de por algum motivo a tarefa ser nula
    if (!task)
        return -1;

    task_t *taskAnterior = taskAtual;
    taskAtual = task;

    if (taskAtual)
        taskAtual->activations++;

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

void task_suspend (task_t **queue) {
   
    // queue_remove(&readyQueue, (queue_t*) taskAtual);

    taskAtual->status = TASK_SUSPENSA;

    // Se uma fila de espera foi fornecida, adiciona a tarefa atual nela
    if (queue)
        queue_append((queue_t**)queue, (queue_t*)taskAtual);

    task_switch(&taskDispatcher);
}

void task_awake (task_t *task, task_t **queue) {
    if (!task) 
        return;

    // Se uma fila foi fornecida, remove a tarefa dela
    if (queue)
        queue_remove((queue_t**)queue, (queue_t*)task);

    task->status = TASK_PRONTA;
    queue_append(&readyQueue, (queue_t*)task);
}

int task_wait (task_t *task) {
    // Impede que uma tarefa espere por si mesma
    if (!task || task == taskAtual || task->status == TASK_TERMINADA)
        return (task ? task->exit_code : -1);

    // Suspende a tarefa atual, colocando-a na fila de espera da tarefa
    task_suspend((task_t**)&task->waiting_queue);

    // Quando for acordada, a tarefa já terá terminado. Retorna seu código de saída.
    return task->exit_code;
}

void task_exit (int exit_code) {

    taskAtual->execution_time = systime() - taskAtual->execution_time;

    printf("Task %d exit: execution time %u ms, processor time %u ms, %u activations\n", task_id(), taskAtual->execution_time, taskAtual->processor_time, taskAtual->activations);

    taskAtual->exit_code = exit_code;
    taskAtual->status = TASK_TERMINADA;

    // Acorda todas as tarefas na fila de espera desta tarefa
    while (taskAtual->waiting_queue) {
        task_t *waiting_task = taskAtual->waiting_queue;

        // Acorda a tarefa, remove da waiting_queue, marca como pronta e coloca na readyQueue
        task_awake(waiting_task, &taskAtual->waiting_queue);
    }

    if (taskAtual == &taskDispatcher) {
        task_switch(&taskMain);
    }
    else if (taskAtual == &taskMain) {
        task_switch(&taskDispatcher);
    }
    else
    {
        userTasks--;
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

unsigned int systime () {
    return system_clock;
}