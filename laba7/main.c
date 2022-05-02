#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include "struct.h"
#include <signal.h>
#include <math.h>

typedef struct
{
    pid_t* array;
    int size;
}array_pid;


array_pid* init_array_pid ();
void create_procces (array_pid ** temp_array, int type_process, const char* walk, const char* type);
void term_procces (array_pid ** temp_array, int type_procces, const char* type);
void term_all (array_pid ** consumers, array_pid ** producers);
char* get_string_from_int (int value);
void make_message (message * create_message_temp);
int rand_int (int max);
int poww (int value, int range);


int main()
{
    ring_buffer * main_buffer = (ring_buffer *) malloc (sizeof (ring_buffer));
    array_pid* produced_pid = init_array_pid();
    array_pid* consumer_pid = init_array_pid();
    pid_t pid_temp;
    int is_exit;
    main_buffer = init_struct ();
    int shmid = shmget (10, sizeof (ring_buffer*), IPC_CREAT | 0666);
    int semid = semget (20, 1, IPC_CREAT | 0666);
    main_buffer = (ring_buffer*) shmat (shmid, NULL, 0);
    semctl (semid, 0, SETVAL, 1);
    int input;
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
                    create_procces (&produced_pid, 0, "./produced", "Производитель");
                    break;
                case 50:
                    term_procces (&produced_pid, 0, "Производитель");
                    break;
                case 51:
                    create_procces (&consumer_pid, 1, "./consumer", "Потребитель");
                    break;
                case 52:
                    term_procces (&consumer_pid, 1, "Потребитель");
                    break;
                case 53:
                    term_all(&consumer_pid, &produced_pid);
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
    semctl (semid, 0, IPC_RMID, NULL);
    shmctl (shmid, IPC_RMID, NULL);
    shmdt (main_buffer);
    return 0;
}

array_pid* init_array_pid ()
{
    array_pid* init_temp = (array_pid*) malloc (sizeof (array_pid));
    init_temp->size = 0;
    init_temp->array = NULL;
    return init_temp;
}

void create_procces (array_pid ** temp_array, int type_process, const char * walk, const char* type)
{
    char** args = malloc (sizeof(char*));
    args[0] = get_string_from_int ((*temp_array)->size);
    pid_t pid_temp = fork();
    if (pid_temp == 0)
        execv (walk, args);
    else
    {
        printf ("%s %d создан с PID %d\n", type, (*temp_array)->size, pid_temp);
        (*temp_array)->array = (pid_t *) realloc ((*temp_array)->array, (++(*temp_array)->size) * sizeof (pid_t));
        (*temp_array)->array[(*temp_array)->size - 1] = pid_temp;
    }
}

void term_procces (array_pid ** temp_array, int type_procces, const char* type)
{
    pid_t pid_temp = (*temp_array)->array[--(*temp_array)->size];
    (*temp_array)->array = (pid_t *) realloc ((*temp_array)->array, (*temp_array)->size * sizeof (pid_t));
    kill (pid_temp, SIGTERM);
    printf ("%s %d с PID %d был удален\n", type, (*temp_array)->size + 1, pid_temp);
}

void term_all (array_pid ** consumers, array_pid ** producers)
{
    for (int i = 0; i < (*consumers)->size; i++)
        kill ((*consumers)->array[i], SIGTERM);
    *consumers = NULL;
    for (int i = 0; i < (*producers)->size; i++)
        kill ((*producers)->array[i], SIGTERM);
    *producers = NULL;
    printf ("Все процессы были удалены\n");
}

char* get_string_from_int (int value)
{
    int range = 0;
    int sub_value = value;
    while (sub_value > 0)
    {
        sub_value /= 10;
        range++;
    }
    char* result = (char*) malloc (range);
    for (int i = range; i > 0; i--)
    {
        result[range - i] = (char)(48 + (int)((value / poww (10, i - 1))) % 10);
    }
    return result;
}

int poww (int value, int range)
{
    int result;
    if (range == 0)
        result = 1;
    else
        if (range == 1)
            result = value;
        else
        {
            result = value;
            for (int i = 1; i < range; i++)
                result *= value;
        }
    return result;
}