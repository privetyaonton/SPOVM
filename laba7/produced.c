#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "struct.h"
#include <signal.h>
#include <time.h>

int count_term;
void term_proc (int sig);
void make_message (message * create_message_temp);
int rand_int (int max);
int push_message_ring_buffer (ring_buffer ** push_ring_buffer_temp, message * push_message_temp);


int main(int argc, char** argv)
{
    count_term = 0;
    int semid = semget (20, 1, 0666);
    int shmid = shmget (10, sizeof(ring_buffer*), 0666);
    ring_buffer* buffer_produced = init_struct();
    buffer_produced = (ring_buffer*) shmat (shmid, NULL, 0);
    signal (SIGTERM, term_proc);
    int is_exec;
    message ms;
    struct sembuf sops[1];
    while (!count_term)
    {
        is_exec = 0;
        sleep (5);
        make_message (&ms);
        if (semctl (semid, 0, GETVAL) == 0)
            continue;
        semctl (semid, 0, SETVAL, 0);
        if (ms.length + buffer_produced->size < MAX_SIZE_BUFFER)
            is_exec = push_message_ring_buffer (&buffer_produced, &ms);
        semctl (semid, 0, SETVAL, 1);
        if (is_exec)
        {
            print_message (&ms, "Производитель", "сгенерировал сообщение и вставил в буфер", atoi (argv[0]));
            printf ("Количество добавленных сообщений - %d\n", buffer_produced->value_in);
        }
    }
    shmdt (buffer_produced);
    exit (0);
}

void term_proc (int sig)
{
    count_term = 1;
}

void make_message (message * create_message_temp)
{
    int size = rand_int (256);
    int type;
    if (size == 256)
        type = 128;
    else
        type = rand_int(127);
    unsigned char * data = malloc (size * sizeof (unsigned char));
    for (int i = 0; i < size; i++)
        data[i] = (unsigned char) rand_int (257);
    int check_sum = 0;
    unsigned char * hash = malloc (2 * sizeof (unsigned char));
    for (int i = 0; i < size; i++)
        check_sum += (int)data[i];
    check_sum %= 510;
    if (check_sum > 255)
    {
        hash[0] = (unsigned char) 255;
        hash[1] = (unsigned char) (check_sum - 255);
    }
    else
        {
            hash[0] = check_sum;
            hash[1] = 0;
        }
    create_message_temp->data[0] = (unsigned char)type;
    memcpy (create_message_temp->data + 1, hash, 2);
    create_message_temp->data[3] = (unsigned char)size;
    memcpy (create_message_temp->data + 4, data, size);
    create_message_temp->length = size + 4;
}

int rand_int (int max)
{
    struct timespec temp_time;
    clock_gettime (CLOCK_REALTIME, &temp_time);
    srand (temp_time.tv_nsec);
    return rand() % (max + 1);
}

int push_message_ring_buffer (ring_buffer ** push_ring_buffer_temp, message * push_message_temp)
{
    int first_half, second_half;
    if (MAX_SIZE_BUFFER - (*push_ring_buffer_temp)->last_index < push_message_temp->length)
    {
        first_half = MAX_SIZE_BUFFER - (*push_ring_buffer_temp)->last_index;
        second_half = push_message_temp->length - first_half;
        memcpy ((*push_ring_buffer_temp)->message + (*push_ring_buffer_temp)->last_index, push_message_temp->data, first_half);
        memcpy ((*push_ring_buffer_temp)->message, push_message_temp->data + first_half, second_half);
        (*push_ring_buffer_temp)->last_index = second_half;
    }
    else
    {
        memcpy ((*push_ring_buffer_temp)->message + (*push_ring_buffer_temp)->last_index, push_message_temp->data, 
            push_message_temp->length);
        (*push_ring_buffer_temp)->last_index += push_message_temp->length;
    }
    (*push_ring_buffer_temp)->value_in++;
    (*push_ring_buffer_temp)->size += push_message_temp->length;
    return 1;
}