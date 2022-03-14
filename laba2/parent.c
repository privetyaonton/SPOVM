#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int compareStr (const char * str1, const char* str2);

char* goToSymbol (int val);

int main(int argc, const char* argv[], const char* envp[])
{
    int i = 0;
    if (argc < 2){
        printf ("Неверное количество аргументов");
        return 0;
    }
    int childCount = 0;
    pid_t pidChild;
    while (envp[i]) //нахождение количества переменных среды
        i++;
    int length_variables = i;
    extern char** environ;
    qsort(envp, length_variables, sizeof(char*), compareStr);
    for (int i = 0; i < length_variables; i++)
        printf ("%s\n", envp[i]); //сортировка и вывод переменных среды
    char* control_charachter = malloc (1);
    char** mas_line_options = malloc (3 * sizeof(char*));
    mas_line_options[0] = malloc(8);
    mas_line_options[1] = malloc(strlen (argv[1]));
    strcpy (mas_line_options[1], argv[1]);
    mas_line_options[2] = malloc(1);
    strcpy (mas_line_options[1], argv[1]);
    while (1)
    {
        printf ("Введите символ: ");
        scanf("%s", control_charachter);
        strcpy (mas_line_options[2], control_charachter);
        if (*control_charachter == '+' || *control_charachter == '*' || *control_charachter == '&' || *control_charachter == '!')
        {
            switch (*control_charachter) //обработка введенного символа
            {
                case '+':
                {
                    pidChild = fork();
                    strcpy (mas_line_options[0], "child_");
                    strcat (mas_line_options[0], goToSymbol(childCount));
                    char* str = malloc (100);
                    if (pidChild == 0){
                        printf ("Дочерний процесс создан...\n");
                        execve(getenv("CHILD_PATH"), mas_line_options, envp);
                    }
                    wait(NULL);
                    printf ("Дочерний процесс завершен...\n");
                    childCount++;
                    break;
                }
                case '*':
                {
                    char* strVariable = malloc (1);
                    int lenStr;
                    for (int i = 0; i < length_variables; i++)
                    {
                        strVariable = NULL;
                        lenStr = 0;
                        while (envp[i][lenStr] != '=')
                        {
                            lenStr++;
                            strVariable = realloc (strVariable, lenStr);
                            strVariable[lenStr - 1] = envp[i][lenStr - 1];
                        }
                        if (strcmp(strVariable, "CHILD_PATH") == 0)
                        {
                            lenStr++;
                            strVariable = NULL;
                            int count = 0;
                            while (envp[i][lenStr + count - 1] != '\0')
                            {
                                count++;
                                strVariable = realloc(strVariable, count);
                                strVariable[count - 1] = envp[i][lenStr + count - 1];
                            }
                            break;
                        }
                    }
                    pidChild = fork();
                    strcpy (mas_line_options[0], "child_");
                    strcat (mas_line_options[0], goToSymbol(childCount));
                    if (pidChild == 0){
                        printf ("Дочерний процесс создан...\n");
                        printf ("%s\n", strVariable);
                        execve(strVariable, mas_line_options, (char* const*)envp);
                    }
                    wait(NULL);
                    printf ("Дочерний процесс завершен...\n");
                    childCount++;
                    break;
                }
                case '&':
                {
                    char* strVariable = malloc (1);
                    int lenStr;
                    for (int i = 0; i < length_variables; i++)
                    {
                        strVariable = NULL;
                        lenStr = 0;
                        while (environ[i][lenStr] != '=')
                        {
                            lenStr++;
                            strVariable = realloc (strVariable, lenStr);
                            strVariable[lenStr - 1] = environ[i][lenStr - 1];
                        }
                        if (strcmp(strVariable, "CHILD_PATH") == 0)
                        {
                            lenStr++;
                            strVariable = NULL;
                            int count = 0;
                            while (environ[i][lenStr + count - 1] != '\0')
                            {
                                count++;
                                strVariable = realloc(strVariable, count);
                                strVariable[count - 1] = environ[i][lenStr + count - 1];
                            }
                            break;
                        }
                    }
                    pidChild = fork();
                    strcpy (mas_line_options[0], "child_");
                    strcat (mas_line_options[0], goToSymbol(childCount));
                    if (pidChild == 0){
                        printf ("Дочерний процесс создан...\n");
                        execve(strVariable, mas_line_options, (char* const*)envp);
                    }
                    wait(NULL);
                    printf ("Дочерний процесс завершен...\n");
                    childCount++;
                    break;
                }
                case '!':
                {
                    printf ("Родительский процесс завершен...\n");
                    exit (0);
                }
            }
        }
        else
        {
            printf ("Неправильный ввод символа\n");
            continue;
        }
    }
}

int compareStr (const char* str1, const char* str2)
{
    return strcmp (str1,str2);
}

char* goToSymbol (int val)
{
    int dec = val / 10;
    int units = val % 10;
    char* str = malloc (2);
    char c_dec = (char)(48 + dec);
    char c_units = (char)(48 + units);
    str[0] = c_dec;
    str[1] = c_units;
    return str;
}
