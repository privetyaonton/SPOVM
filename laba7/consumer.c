#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "struct.h"
#include <signal.h>

int count_term;
void term_proc (int sig);
int pop_ring_buffer (ring_buffer ** pop_ring_buffer_temp, message * message_buffer);

int main(int argc, char** argv)
{
    count_term = 0;
    int semid = semget (20, 3, 0666);
    int shmid = shmget (10, sizeof (ring_buffer*), 0666);
    ring_buffer* buffer_consumer = init_struct ();
    buffer_consumer = (ring_buffer*) shmat (shmid, NULL, 0);
    signal (SIGTERM, term_proc);
    message ms;
    int is_exec;
    struct sembuf sops[1];
    while (!count_term)
    {
        is_exec = 0;
        sleep (5);
        if (semctl (semid, 0, GETVAL) == 0)
            continue;
        semctl (semid, 0, SETVAL, 0);
        if (buffer_consumer->size > 0)
            is_exec = pop_ring_buffer (&buffer_consumer, &ms);
        semctl (semid, 0, SETVAL, 1);
        if (is_exec)
        {
            print_message (&ms, "Потребитель", "извлек сообщение", atoi (argv[0]));
            printf ("Количество извлеченных сообщений - %d\n", buffer_consumer->value_out);
        }
    }
    shmdt (buffer_consumer);
    exit (0);
}

void term_proc (int sig)
{
    count_term = 1;
}

int pop_ring_buffer (ring_buffer ** pop_ring_buffer_temp, message * message_buffer)
{
    int first_half, second_half;
    message_buffer->length = 0;
    if (MAX_SIZE_BUFFER - (*pop_ring_buffer_temp)->first_index < 4)
    {
        first_half = MAX_SIZE_BUFFER - (*pop_ring_buffer_temp)->first_index;
        second_half = 4 - first_half;
        memcpy (message_buffer->data, (*pop_ring_buffer_temp)->message + (*pop_ring_buffer_temp)->first_index, first_half);
        memcpy (message_buffer->data + first_half, (*pop_ring_buffer_temp)->message, second_half);
        (*pop_ring_buffer_temp)->first_index = second_half;
    }
    else
    {
        memcpy (message_buffer->data, (*pop_ring_buffer_temp)->message + (*pop_ring_buffer_temp)->first_index, 4);
        (*pop_ring_buffer_temp)->first_index += 4;
    }
    int size = (int)message_buffer->data[3];
    if (MAX_SIZE_BUFFER - (*pop_ring_buffer_temp)->first_index < size)
    {
        first_half = MAX_SIZE_BUFFER - (*pop_ring_buffer_temp)->first_index;
        second_half = size - first_half;
        memcpy (message_buffer->data + 4, (*pop_ring_buffer_temp)->message + (*pop_ring_buffer_temp)->first_index, first_half);
        memcpy (message_buffer->data + 4 + first_half, (*pop_ring_buffer_temp)->message, second_half);
        (*pop_ring_buffer_temp)->first_index = second_half;
    }
    else
    {
        memcpy (message_buffer->data + 4, (*pop_ring_buffer_temp)->message + (*pop_ring_buffer_temp)->first_index, size);
        (*pop_ring_buffer_temp)->first_index += size;
    }
    message_buffer->length = size + 4;
    (*pop_ring_buffer_temp)->size -= message_buffer->length;
    (*pop_ring_buffer_temp)->value_out++;
    return 1;
}