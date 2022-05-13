#include "FileSystem.h"

typedef struct
{
    char com [10];
    char arg [200];
    char sub_arg[200];
}command;

int recv_all (int sockfd, void* buffer, int len);
char** list_command = {
    "cd", "ls", "home", "cat", "exit", "help", "loaddir", "loadfile"
};
char* manual1 = {"cd <dir> - перейти к каталогу\nls - просмотреть объекты в каталоге\nhome - перейти в домашний каталог\n"};
char* manual2 = {"cat <filename> - вывести на экран содержимое файла\nexit - выйти\nhelp - вызвать мануал\n"};
char* manual3 = {"loaddir <dirname> <walkload> - скачать каталог\nloadfile <filename> <walkload> - скачать файл\n"};
int size_list_command = 5;
struct my_dir * execute_command (command * pack_command, struct my_dir * dir);
void recv_objects (struct my_dir * home, int sockfd);
int is_exit = 0;
void* parse_command ();
struct my_dir * home;
void load_dir (char* work_walk, struct my_dir * dir_load);
void load_file (char* walk_load, struct file * load_file);

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        printf ("Неверное количество аргументов\n");
        return 0;
    }
    home = (struct my_dir*) malloc (sizeof (struct my_dir));
    if (home == NULL)
    {
        printf ("Ошибка при выделении памяти\n");
        exit (0);
    }
    home->size_dirs = 0;
    home->size_objects = 0;
    int client_sock_fd;
    struct sockaddr_in client_info;
    client_sock_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (client_sock_fd < 0)
    {
        printf ("Ошибка при создании сокета\n");
        return 0;
    }
    client_info.sin_addr.s_addr = inet_addr (argv[1]);
    client_info.sin_family = AF_INET;
    client_info.sin_port = htons (8888);
    if (connect (client_sock_fd, (struct sockaddr *)&client_info, sizeof (client_info)) < 0)
    {
        printf ("Ошибка при соединении\n");
        return 0;
    }
    pthread_t thread;
    pthread_create (&thread, NULL, parse_command, NULL);
    pthread_join (thread, NULL);
    while (!is_exit)
        recv_objects (home, client_sock_fd);
    return 0;
}

int recv_all (int sockfd, void* buffer, int len)
{
    char* pbuf = (char*) buffer;
    int total = 0;
    while (len > 0)
    {
        int rval = recv(sockfd, pbuf, len, 0);
        if (rval < 0)
        {
            printf("Ошибка чтения сокета\n");
            return -1;
        }
        if (rval == 0)
            break;
        pbuf += rval;
        len -= rval;
        total += rval;
    }
    return total;
}

struct my_dir * execute_command (command * pack_command, struct my_dir * dir)
{
    int cont = 0;
    for (int i = 0; i < size_list_command; i++)
    {
        if (strcmp (pack_command->com, list_command[i]) == 0)
        {
            cont = 1;
            break;
        }
    }
    if (!cont)
    {
        printf ("Неверная команда\n");
        return dir;
    }
    if (strcmp (pack_command->com, "cd") == 0)
    {
        for (int i = 0; i < dir->size_dirs; i++)
            if (strcmp(dir->dirs[i].name_dir, pack_command->arg) == 0)
                return &dir->dirs[i].dirs[i];
        printf ("Такой директории не найдено\n");
        return dir;
    }
    if (strcmp (pack_command->com, "ls") == 0)
    {
        for (int i = 0; i < dir->size_dirs; i++)
            printf ("%s\n", dir->dirs[i].name_dir);
        for (int i = 0; i < dir->size_objects; i++)
            printf ("%s\n", dir->objects[i].name_file);
        return dir;
    }
    if (strcmp (pack_command->com, "home") == 0)
        return home;
    if (strcmp (pack_command->com, "cat") == 0)
    {
        int number_spaces = calculating_space (pack_command->arg);
        char name_file[strlen (pack_command->arg) + number_spaces];
        if (number_spaces != 0)
        {
            int i = 0;
            int size_buf = strlen(name_file);
            while (i < size_buf)
            {
                if (name_file[i] == ' ')
                {
                    size_buf++;
                    for (int j = size_buf - 1; j > i; j--)
                        name_file[j] = name_file[j - 1];
                    name_file[i] = (char) 92;
                    i += 2;
                    continue;
                }
                i++;
            }
        }       
        char main_walk[500];
        getcwd (main_walk, 500);
        strcat (main_walk, "/");
        strcat (main_walk, pack_command->arg);
        struct file temp;
        convert_file_to_data (main_walk, pack_command->arg, &temp, name_file);
        printf ("%s\n", temp.data_file);
        return dir;
    }
    if (strcmp (pack_command->com, "exit") == 0)
    {
        is_exit = 1;
        return dir;
    }
    if (strcmp (pack_command->com, "loadfile") == 0)
    {
        for (int i = 0; i < dir->size_objects; i++)
            if (strcmp(dir->objects[i].name_file, pack_command->arg) == 0)
            {
                load_file (pack_command->sub_arg, &dir->objects[i]);
                return dir;
            }
        printf ("Такого файла нет в этом каталоге\n");
        return dir;
    }
    if (strcmp (pack_command->com, "loaddir") == 0)
    {
        for (int i = 0; i < dir->size_dirs; i++)
            if (strcmp (dir->dirs[i].name_dir, pack_command->arg) == 0)
            {
                char work_walk[500];
                getcwd (work_walk, 500);
                load_dir (work_walk, &dir->dirs[i]);
                move_file (pack_command->sub_arg, pack_command->arg);
                return dir;
            }
        printf ("Такого каталога не существует\n");
    }
    return dir;
}

void recv_objects (struct my_dir * home, int sockfd)
{
    recv_all (sockfd, &home->size_dirs, sizeof (int));
    for (int i = 0; i < home->size_dirs; i++)
    {
        recv_all (sockfd, &home->dirs[i].size_name_dir, sizeof (int));
        recv_all (sockfd, home->dirs[i].name_dir, strlen (home->dirs[i].name_dir));
        recv_objects (&home->dirs[i], sockfd);
    }
    recv_all (sockfd, &home->size_objects, sizeof (home->size_objects));
    for (int i = 0; i < home->size_objects; i++)
    {
        recv_all (sockfd, &home->objects[i].type, sizeof (int));
        recv_all (sockfd, &home->objects[i].size_name_file, sizeof (int));
        recv_all (sockfd, home->objects[i].name_file, sizeof (home->objects[i].name_file));
        recv_all (sockfd, &home->objects[i].size_data, sizeof (int));
        recv_all (sockfd, home->objects[i].data_file, sizeof (home->objects[i].data_file));
    }
}

void* parse_command ()
{
    char input_str[1000];
    char first_half[100];
    char second_half[100];
    char three_half[100];
    int count;
    struct my_dir * dir = home;
    while (!is_exit)
    {
        system ("clear");
        printf ("%s%s%s", manual1, manual2, manual3);
        scanf ("%s", input_str);
        int spaces = calculating_space (input_str);
        if (spaces > 2)
        {
            printf ("Неверный ввод\n");
            continue;
        }
        command pack_command;
        if (spaces == 0)
            strcpy (pack_command.com, input_str);
        else if (spaces == 1)
            {
                count = 0;
                while (input_str[count] != ' ')
                {
                    first_half[count] = input_str[count];
                    count++;
                }
                count++;
                for (int i = count; i < strlen (input_str); i++)
                {
                    second_half [i - count] = input_str[count];
                }
                strcpy (pack_command.com, first_half);
                strcpy (pack_command.arg, second_half);
            }
            else
            {
                count = 0;
                while (input_str[count] != ' ')
                {
                    first_half[count] = input_str[count];
                    count++;
                }
                count++;
                int i = 0;
                while (input_str[count] != ' ')
                {
                    second_half[i] = input_str[count];
                    count++;
                    i++;
                }
                count++;
                for (int i = count; i < strlen (input_str); i++)
                {
                    three_half [i - count] = input_str[count];
                }
                strcpy (pack_command.com, first_half);
                strcpy (pack_command.arg, second_half);
                strcpy (pack_command.sub_arg, three_half);
            }
        dir = execute_command (&pack_command, dir);
    }
    return NULL;
}

void load_file (char* walk_load, struct file * load_file)
{
    FILE* file = fopen (load_file->name_file, "wb");
    fwrite (load_file->data_file, 1, load_file->size_data, file);
    fclose (file);
    move_file (walk_load, load_file->name_file);
}

void load_dir (char* work_walk, struct my_dir * dir_load)
{
    char mkdir[500];
    char* stack = malloc (strlen (work_walk));
    strcpy (stack, work_walk);
    strcpy (mkdir, "mkdir ");
    strcat (mkdir, dir_load->name_dir);
    system (mkdir);
    move_file (work_walk, dir_load->name_dir);
    strcat (work_walk, "/");
    strcat (work_walk, dir_load->name_dir);
    for (int i = 0; i < dir_load->size_objects; i++)
        load_file (work_walk, &dir_load->objects[i]);
    for (int i = 0; i < dir_load->size_dirs; i++)
        load_dir (work_walk, &dir_load->dirs[i]);
}
