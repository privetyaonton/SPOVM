#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[])
{
    extern char** environ;
    printf ("Дочерний процесс запущен...\n");
    printf ("имя - %s\npid - %d\nppid - %d\n", argv[0], (int)getpid(), (int)getppid());
    FILE * file = fopen (argv[1], "rt");
    char** masStr = malloc (sizeof(char*));
    char* str = malloc (1000); 
    int height_mas = 0;
    rewind (file);
    while (!feof(file))
    {
        fscanf (file, "%s\n", str);
        height_mas++;
        masStr = realloc (masStr, height_mas * sizeof(char*));
        masStr[height_mas - 1] = malloc (strlen (str));
        strcpy (masStr[height_mas - 1], str);
    }
    switch (*(argv[2]))
    {
        case '+':
        {
            for (int i = 0; i < height_mas; i++)
                printf ("%s=%s\n", masStr[i], getenv(masStr[i]));
            break;
        }
        case '*':
        {
            int index = 0;
            int var_index;
            int name_index;
            while (envp[index])
            {
                var_index = 0;
                str = NULL;
                while (envp[index][var_index] != '=')
                    var_index++;
                var_index++;
                name_index = 0;
                while (envp[index][var_index + name_index - 1] != '\0')
                {
                    name_index++;
                    str = realloc(str, name_index);
                    str[name_index - 1] = envp[index][var_index + name_index - 1];
                }
                printf ("%s=%s\n", masStr[index], str);
                index++;
            }
            break;
        }
        case '&':
        {
            int index = 0;
            int var_index;
            int name_index;
            while (environ[index])
            {
                var_index = 0;
                str = NULL;
                while (environ[index][var_index] != '=')
                    var_index++;
                var_index++;
                name_index = 0;
                while (environ[index][var_index + name_index - 1] != '\0')
                {
                    name_index++;
                    str = realloc(str, name_index);
                    str[name_index - 1] = environ[index][var_index + name_index - 1];
                }
                printf ("%s=%s\n", masStr[index], str);
                index++;
            }
            break;
        }
    }
    exit(0);
}
