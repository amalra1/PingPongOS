/*

Nome: Pedro Amaral Chapelin

GRR: 20206145

*/

#include <stdio.h>
#include "queue.h"

#define ERR_FILA_NAO_EXISTE -1;
#define ERR_ELEM_NULO -2;
#define ERR_ELEM_EM_OUTRA_FILA -3;
#define ERR_FILA_VAZIA -4;
#define ERR_ELEM_NAO_PERTENCE_A_FILA -5;

int queue_size (queue_t *queue)
{
    // Fila vazia
    if (!queue)
        return 0;

    int count = 1;
    queue_t *current = queue->next;

    while (current != queue)
    {
        count++;
        current = current->next;
    }

    return count;
}

int queue_append (queue_t **queue, queue_t *elem)
{
    // Vendo se a fila existe
    if (!queue)
    {
        fprintf(stderr, "A fila não existe.\n");
        return ERR_FILA_NAO_EXISTE;
    }

    // Vendo se o elemento não é NULL
    if (!elem)
    {
        fprintf(stderr, "O elemento a ser adicionado é nulo.\n");
        return ERR_ELEM_NULO;
    }

    // Vendo se o elemento já está em outra fila
    if (elem->prev != NULL || elem->next != NULL)
    {
        fprintf(stderr, "O elemento já pertence a uma fila.\n");
        return ERR_ELEM_EM_OUTRA_FILA;
    }

    // Fila vazia, o elemento recebe os dois ponteiros
    if (*queue == NULL)
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
    }
    // Fila não vazia, atualiza também os ponteiros do elem anterior e do topo, já que é circular
    else
    {
        queue_t *head = *queue;
        queue_t *tail = head->prev;

        tail->next = elem;
        elem->prev = tail;
        elem->next = head;
        head->prev = elem;
    }

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem)
{
    // Vendo se a fila existe
    if (!queue)
    {
        fprintf(stderr, "A fila não existe.\n");
        return ERR_FILA_NAO_EXISTE;
    }

    // Vendo se o elemento não é NULL
    if (!elem)
    {
        fprintf(stderr, "O elemento a ser adicionado é nulo.\n");
        return ERR_ELEM_NULO;
    }

    // Vendo se a fila está vazia
    if (*queue == NULL)
    {
        fprintf(stderr, "A fila não pode estar vazia.\n");
        return ERR_FILA_VAZIA;
    }

    // Vendo se o elemento a ser removido pertence a fila
    queue_t* current;
    int belongs = 0;

    // Caso dele ser o topo da fila
    if (*queue == elem)
        belongs = 1;
    else 
    {
        current = (*queue)->next;

        while (current != *queue) 
        {
            if (current == elem) 
            {
                belongs = 1;
                break;
            }
            current = current->next;
        }
    }
    
    if (!belongs) 
    {
        fprintf(stderr, "O elemento não pertence à fila indicada.\n");
        return ERR_ELEM_NAO_PERTENCE_A_FILA;
    }

    // Se for o único da fila
    if (elem->next == elem)
        *queue = NULL; 
    else
    {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;

        // Se for o topo da fila
        if (*queue == elem)
            *queue = elem->next;
    }

    elem->prev = NULL;
    elem->next = NULL;

    return 0;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
    printf("%s: [", name);

    if (queue)
    {
        int size = queue_size(queue);
        queue_t* current = queue;

        for (int i = 0; i < size; i++)
        {
            print_elem(current);

            if (i < size - 1)
                printf(" ");

            current = current->next;
        }
    }

    printf("]\n");
}