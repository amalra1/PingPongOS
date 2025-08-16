/*

Nome: Pedro Amaral Chapelin

GRR: 20206145

*/

#include <stdio.h>
#include "queue.h"

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
    // Vendo se a fila não é NULL
    if (!queue)
    {
        fprintf(stderr, "A fila é nula.\n");
        return -1;
    }

    // Vendo se o elemento não é NULL
    if (!elem)
    {
        fprintf(stderr, "O elemento a ser adicionado é nulo.\n");
        return -2;
    }

    // Vendo se o elemento já está em outra fila
    if (elem->prev != NULL || elem->next != NULL)
    {
        fprintf(stderr, "O elemento já pertence a uma fila.\n");
        return -3;
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