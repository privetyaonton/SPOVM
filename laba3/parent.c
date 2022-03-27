#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

char* convertInt (int value);
void acceptPrint (int sig);
pid_t* pid;
int count_pid;

int main()
{
    count_pid = 0;
    pid_t temp;
    pid = NULL;
    signal (SIGALRM, acceptPrint);
    printf ("Родительский процесс запущен\n");
    char input_string;
    int value_child;
    char* walk_child = malloc (500);
    strcpy (walk_child, getenv("CHILD_PATH"));
    bool isEnter;
    while (1)
    {
        fflush (stdin);
        scanf ("%c", &input_string);
        isEnter = false;
        switch (input_string)
        {
            case '+': //порождение процесса
                count_pid++;
                temp = fork();
                if (temp)
                {
                    printf ("%d\n", count_pid); 
                    pid = realloc(pid, count_pid * sizeof (pid_t));
                    printf ("Дочерний процесс C_%d создан\n", count_pid - 1);
                    printf ("PID - %d\n", temp);
                    pid[count_pid - 1] = temp;
                }
                else
                    execv ("child", NULL);
                isEnter = true;
                break;
            case '-': //удаление последнего процесса
                kill (pid[count_pid - 1], SIGKILL);
                count_pid--;
                pid = realloc (pid, count_pid * sizeof(pid_t));
                printf ("Дочерний процесс C_%d удален\n", count_pid);
                isEnter = true;
                break;
            case 'k': //удаление всех процессов
                for (int i = 0; i < count_pid; i++)
                    kill (pid[i], SIGKILL);
                pid = NULL;
                count_pid = 0;
                printf ("Все дочерние процессы были удалены\n");
                isEnter = true;
                break;
            case 's': //запрет вывода
                printf ("Введите номер дочернего процесса, для запрета всех введите -1 - ");
                scanf ("%d", &value_child);
                if (value_child != -1)
                    kill (pid[value_child], SIGUSR1);
                else
                    for (int i = 0; i < count_pid; i++)
                        kill (pid[i], SIGUSR1);
                isEnter = true;
                break;
            case 'g': //разрешение вывода
                printf ("Введите номер дочернего процесса, для разрешения всех введите -1 - ");
                scanf ("%d", &value_child);
                if (value_child != -1)
                    kill (pid[value_child], SIGUSR2);
                else
                    for (int i = 0; i < count_pid; i++)
                        kill(pid[i], SIGUSR2);
                isEnter = true;
                break;
            case 'p': //разрешение одному из С_num 
                printf ("Введите номер дочернего процесса - ");
                scanf ("%d", &value_child);
                for (int i = 0; i < count_pid; i++)
                    if (i != value_child)
                        kill (pid[i], SIGUSR1);
                    else
                        kill (pid[i], SIGINT);
                alarm (5);
                isEnter = true;
                break;
            case 'q': //удаление всех процессов и завершение род.
                for (int i = 0; i < count_pid; i++)
                    kill (pid[i], SIGKILL);
                pid = NULL;
                return 0;
                break;
        }
    }
    return 0;
}

char* convertInt (int value) //переводит число в строку
{
    char* str = malloc (2);
    if (value / 10 > 0)
    {
        str[0] = (char)(value/10 + 48);
        str[1] = (char)(value%10 + 48);
    }
    else
        str[0] = (char)(value + 48);
    return str;
}
void acceptPrint (int sig)
{
    for (int i = 0; i < count_pid; i++)
        kill (pid[i], SIGUSR2);
}