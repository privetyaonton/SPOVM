#ifndef STRUCT_H
#define STRUCT_H
#define MAX_SIZE_BUFFER_MESSAGE 300
#define MAX_SIZE_BUFFER 3000
#include <sys/sem.h>

typedef struct 
{
    unsigned char message[MAX_SIZE_BUFFER];
    int first_index;
    int last_index;
    int value_in;
    int value_out;
    int size;
}ring_buffer;

typedef struct 
{
    int length;
    unsigned char data[MAX_SIZE_BUFFER_MESSAGE];
}message;

ring_buffer* init_struct ()
{
    ring_buffer* init_temp = (ring_buffer*) malloc (sizeof (ring_buffer));
    init_temp->size = 0;
    init_temp->first_index = init_temp->last_index = init_temp->value_in = init_temp->value_out = 0;
    return init_temp;
}

void print_message (message * print_message_temp, const char* type_proc, const char* type_move, int number_proc)
{
    printf ("%s %d с PID %d %s\n", type_proc, number_proc, (int)getpid(), type_move);
    printf ("Данные сообщения:\n");
    int k = 0;
    for (int i = 0; i < print_message_temp->length; i++)
    {
        if (k == 14)
        {
            printf ("\n");
            k = 0;
        }
        else
        {
            printf ("%d ", (int)print_message_temp->data[i]);
            k++;
        }
    }
    printf ("\n");
}

#endif