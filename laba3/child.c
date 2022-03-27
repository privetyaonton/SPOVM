#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

struct Values
{
    int first_value;
    int second_value;
}mem;

bool allow;
int array_pairs[4];
sig_atomic_t signal_exit;
void allow_print (int sign);
void ban_print (int sign);
void inquiry_print (int sign);
void check_struct (int sign);
void print ();
int getNumber (char* str);

int main()
{
    struct Values zeros = {0, 0};
    struct Values ones = {1,1};
    signal (SIGALRM, check_struct);
    signal (SIGUSR1, ban_print);
    signal (SIGUSR2, allow_print);
    signal (SIGINT, inquiry_print);
    int count = 0;
    while (1)
    {
        signal_exit = 0;
        if (count >= 10 && allow)
        {
            print();
            count = 0;
        }
        alarm (1);
        usleep (9000);
        count++;
        while (!signal_exit)
        {
            mem = zeros;
            nanosleep (0, 20);
            mem = ones;
        }
    }
}

void allow_print (int sign)
{
    allow = true;
}

void ban_print (int sign)
{
    allow = false;
}

void inquiry_print (int sign)
{
    print();
}

void print()
{
    printf("PID - %d\nPPID - %d\n00 - %d\n01 - %d\n10 - %d\n11 - %d\n",
        getpid(), getppid(), array_pairs[0], array_pairs[1], array_pairs[2],
            array_pairs[3]);
}

int getNumber (char* str)
{
    int i = 2;
    char* number = malloc(2);
    while (str[i] != '\0')
    {
        number[i - 2] = str[i];
        i++;
    }
    int value_child = atoi (number);
    return value_child;
}

void check_struct (int sign)
{
    signal_exit = 1;
    if (mem.first_value == 0 && mem.second_value == 0)
        array_pairs[0]++;
    if (mem.first_value == 0 && mem.second_value == 1)
        array_pairs[1]++;
    if (mem.first_value == 1 && mem.second_value == 0)
        array_pairs[2]++;
    if (mem.first_value == 1 && mem.second_value == 1)
        array_pairs[3]++;
}