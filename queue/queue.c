/*

Nome: Pedro Amaral Chapelin

GRR: 20206145

*/

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