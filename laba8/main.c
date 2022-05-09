#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define MAX_SIZE_BUFFER_MESSAGE 300
#define MAX_SIZE_BUFFER 3000

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

typedef struct 
{
    pthread_t * array;
    int size;
}array_pthread;

ring_buffer* init_struct ();
ring_buffer * main_buffer;
void print_message (message * print_message_temp, const char* type_move);
pthread_mutex_t mutex_work_buffer;
void* produced ();
void* consumer ();
void make_message (message * create_message_temp);
int rand_int (int max);
int push_message_ring_buffer (ring_buffer ** push_ring_buffer_temp, message * push_message_temp);
int pop_ring_buffer (ring_buffer ** pop_ring_buffer_temp, message * message_buffer);


int main()
{
    pthread_mutex_init (&mutex_work_buffer, NULL);
    main_buffer = (ring_buffer*) malloc (sizeof (ring_buffer));
    main_buffer = init_struct();
    array_pthread * produced_pthread = malloc (sizeof (array_pthread));
    array_pthread * consumer_pthread = malloc (sizeof (array_pthread));
    produced_pthread->size = consumer_pthread->size = 0;
    int is_exit, input;
    while (1)
    {
        printf ("1 - Добавить производитель\n2 - Удалить производитель\n3 - Добавить потребитель\n4 - Удалить производитель\n5 - Выход\n");
        fflush (stdin);
        is_exit = 0;
        input = getchar();
        if (input >= 49 && input <= 53)
        {
            switch (input)
            {
                case 49:
                    produced_pthread->size++;
                    produced_pthread->array = (pthread_t*) realloc (produced_pthread->array, produced_pthread->size * sizeof (pthread_t));
                    pthread_create (&produced_pthread->array[produced_pthread->size - 1], NULL, produced, NULL);
                    printf ("Производитель создан\n");
                    break;
                case 50:
                    produced_pthread->size--;
                    produced_pthread->array = (pthread_t*) realloc (produced_pthread->array, produced_pthread->size * sizeof (pthread_t));
                    pthread_cancel (produced_pthread->array[produced_pthread->size - 1]);
                    printf ("Производитель удален\n");
                    break;
                case 51:
                    consumer_pthread->size++;
                    consumer_pthread->array = (pthread_t*) realloc (consumer_pthread->array, consumer_pthread->size * sizeof (pthread_t));
                    pthread_create (&consumer_pthread->array[consumer_pthread->size - 1], NULL, consumer, NULL);
                    printf ("Потребитель создан\n");
                    break;
                case 52:
                    consumer_pthread->size--;
                    consumer_pthread->array = (pthread_t*) realloc (consumer_pthread->array, consumer_pthread->size * sizeof (pthread_t));
                    pthread_cancel (consumer_pthread->array[consumer_pthread->size - 1]);
                    printf ("Производитель удален\n");
                    break;
                case 53:
                    for (int i = 0; i < produced_pthread->size; i++)
                        pthread_cancel (produced_pthread->array[i]);
                    for (int i = 0; i < consumer_pthread->size; i++)
                        pthread_cancel (consumer_pthread->array[i]);
                    produced_pthread->array = NULL;
                    consumer_pthread->array = NULL;
                    produced_pthread->size = consumer_pthread->size = 0;
                    is_exit = 1;
                    break;
            }
        }
        else
        {
            printf ("Неверный ввод\n");
            continue;
        }
        if (is_exit)
            break;
    }
    pthread_mutex_destroy (&mutex_work_buffer);
}

ring_buffer* init_struct ()
{
    ring_buffer* init_temp = (ring_buffer*) malloc (sizeof (ring_buffer));
    init_temp->size = 0;
    init_temp->first_index = init_temp->last_index = init_temp->value_in = init_temp->value_out = 0;
    return init_temp;
}

void print_message (message * print_message_temp, const char* type_move)
{
    printf ("%s\n", type_move);
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

void * produced ()
{
    message ms;
    int is_exec;
    while (1)
    {
        is_exec = 0;
        sleep (5);
        make_message (&ms);
        pthread_mutex_lock (&mutex_work_buffer);
        if (ms.length + main_buffer->size < MAX_SIZE_BUFFER)
            is_exec = push_message_ring_buffer (&main_buffer, &ms);
        pthread_mutex_unlock (&mutex_work_buffer);
        if (is_exec)
        {
            print_message (&ms, "Производитель сгенерировал сообщение и вставил в буфер");
            printf ("Количество добавленных сообщений - %d\n", main_buffer->value_in);
        }
    }
}

void * consumer ()
{
    message ms;
    int is_exec;
    while (1)
    {
        is_exec = 0;
        sleep (5);
        pthread_mutex_lock (&mutex_work_buffer);
        if (main_buffer->size > 0)
            is_exec = pop_ring_buffer (&main_buffer, &ms);
        pthread_mutex_unlock (&mutex_work_buffer);
        if (is_exec)
        {
            print_message (&ms, "Потребитель извлек сообщение");
            printf ("Количество извлеченных сообщений - %d\n", main_buffer->value_out);
        }
    }
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