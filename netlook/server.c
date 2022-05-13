#include "FileSystem.h"

typedef struct
{
    char com [10];
    char arg [200];
}command;

int send_all(int sockfd, const void * buffer, int len);
int shell_server (char* command);
int tools_socket (int port);
char* manual = {"/help - мануал\n/push - добавить новую папку в облако\n/exit - выйти\n"};
int size_list_command = 5;
void send_objects (struct my_dir * home_dir, int sockfd);
struct my_dir * execute_command (command * pack_command, struct my_dir * dir);
void collect_dir (struct my_dir ** dir, const char* walk_dir);
int set_type (struct dirent* temp_set_type_dirent);


int main()
{
    struct my_dir * home_dir = malloc (sizeof (struct my_dir));
    home_dir->size_dirs = 0;
    home_dir->size_name_dir = 0;
    home_dir->size_objects = 0;
    if (home_dir == NULL)
    {
        printf ("Ошибка при выделении памяти\n");
        exit (0);
    }
    int socket_for_push;
    //socket_for_push = tools_socket(8888);
    char input_str[1000];
    int switch_value;
    while (1)
    {
       // system ("clear");
        printf ("%s", manual);
        scanf ("%s", input_str);
        switch_value = shell_server (input_str);
        if (switch_value == 2)
            break;
        if (switch_value == -1)
        {
            printf ("Неверная команда\n");
            continue;
        }
        if (switch_value == 1)
        {
            printf ("Путь к папке - ");
            fflush (stdin);
            scanf ("%s", input_str);
            collect_dir (&home_dir, input_str);
            send_objects (home_dir, socket_for_push);
        }
    }
}

int send_all(int sockfd, const void * buffer, int len)
{
    const char *pbuf = (const char *) buffer;
    while (len > 0)
    {
        int sent = send(sockfd, pbuf, len, 0);
        if (sent < 1)
        {
            printf ("Не получилось записать в сокет\n");
            return -1;
        }
        pbuf += sent;
        len -= sent;
    }
    return 0;
}

int shell_server (char* command)
{
    if (strcmp (command, "/help") == 0)
        return 0;
    else
        if (strcmp (command, "/exit") == 0)
            return 2;
        else
            if (strcmp (command, "/push") == 0)
                return 1;
            else
                return -1;
}

int tools_socket (int port)
{
    int server_sock_fd, client_sock_fd;
    struct sockaddr_in server_buffer_info;
    server_buffer_info.sin_addr.s_addr = htonl (INADDR_ANY);
    server_buffer_info.sin_family = AF_INET;
    server_buffer_info.sin_port = htons (port);
    server_sock_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (server_sock_fd < 0)
    {
        printf ("Ошибка при создании сокета - Error %d", errno);
        exit (0);
    }
    if (bind (server_sock_fd, (struct sockaddr *)&server_buffer_info, sizeof (server_buffer_info)) < 0)
    {
        printf ("Ошибка при привязывании имени к сокету - Error %d\n", errno);
        exit (0);
    }
    listen (server_sock_fd, 50);
    client_sock_fd = accept (server_sock_fd, NULL, NULL);
    return client_sock_fd;
}

void send_objects (struct my_dir * home_dir, int sockfd)
{
    send_all (sockfd, &home_dir->size_dirs, sizeof (home_dir->size_dirs));
    for (int i = 0; i < home_dir->size_dirs; i++)
    {
        send_all (sockfd, &home_dir->dirs[i].size_name_dir, sizeof (int));
        send_all (sockfd, home_dir->dirs[i].name_dir, sizeof (home_dir->dirs[i].name_dir));
        send_objects (&home_dir->dirs[i], sockfd);
    }
    send_all (sockfd, &home_dir->size_objects, sizeof (home_dir->size_objects));
    for (int i = 0; i < home_dir->size_objects; i++)
    {
        send_all (sockfd, &home_dir->objects[i].type, sizeof (int));
        send_all (sockfd, &home_dir->objects[i].size_name_file, sizeof (int));
        send_all (sockfd, home_dir->objects[i].name_file, sizeof (home_dir->objects[i].name_file));
        send_all (sockfd, &home_dir->objects[i].size_data, sizeof (int));
        send_all (sockfd, home_dir->objects[i].data_file, sizeof (home_dir->objects[i].data_file));
    }
}

void collect_dir (struct my_dir ** dir, const char* walk_dir)
{
    printf ("dadada\n");
    struct dirent * dirent_file;
    int number_spaces;
    DIR* dir_temp = opendir (walk_dir);    
    if (errno == ENOTDIR)
    {
        printf ("Такого каталога не существует\n");
        return;
    }
    (*dir)->size_dirs = 0;
    (*dir)->size_name_dir = 0;
    (*dir)->size_objects = 0;
    while ((dirent_file = readdir (dir_temp)))
    {
        if (strcmp (dirent_file->d_name, ".") != 0 && strcmp (dirent_file->d_name, "..") != 0 && dirent_file->d_name[0] != '.')
        {
            number_spaces = calculating_space (dirent_file->d_name);
            char* name_file = malloc (strlen (dirent_file->d_name) + number_spaces);
            strcpy (name_file, dirent_file->d_name);
            if (strlen(name_file) > MAX_SIZE_NAME_FILE)
            {
                printf ("Слишком большое название у файла: %s/%s", walk_dir, dirent_file->d_name);
                break;
            }
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
            if (dirent_file->d_type == DT_DIR)
            {
                if ((*dir)->size_dirs == MAX_NUMBERS_ENTRIES_DIRS)
                {
                    printf ("Слишком много вложенных директорий в папке: %s/%s", walk_dir, dirent_file->d_name);
                    break;
                }
                struct my_dir * temp_dir;
                temp_dir = (struct my_dir *) malloc (sizeof (struct my_dir));
                temp_dir->size_name_dir = strlen (dirent_file->d_name);
                strcpy (temp_dir->name_dir, dirent_file->d_name);
                char* this_walk = malloc (strlen (walk_dir) + strlen (name_file) + 1);
                strcpy (this_walk, walk_dir);
                strcat (this_walk, "/");
                strcat (this_walk, name_file);
                collect_dir (&temp_dir, this_walk);
                (*dir)->size_dirs++;
                (*dir)->dirs = (struct my_dir *)realloc ((*dir)->dirs, (*dir)->size_dirs * sizeof (struct my_dir));
                memcpy (&(*dir)->dirs[(*dir)->size_dirs - 1], temp_dir, sizeof (struct my_dir));
            }
            if (dirent_file->d_type == DT_REG || dirent_file->d_type == DT_LNK)
            {
                if ((*dir)->size_objects == MAX_NUMBERS_ENTRIES_OBJECTS)
                {
                    printf ("Слишком много вложенных файлов в папке: %s/%s", walk_dir, dirent_file->d_name);
                    break;
                }
                // struct file _file;
                // _file.type = set_type (dirent_file);
                // _file.size_name_file = strlen (dirent_file->d_name);
                // convert_file_to_data (walk_dir, dirent_file->d_name, &_file, name_file);
                // (*dir)->size_objects++;
                // (*dir)->objects = (struct file *) realloc ((*dir)->objects, (*dir)->size_objects * sizeof (struct file));
                // (*dir)->objects[(*dir)->size_objects - 1] = _file;
            }
        }
    }
}

int set_type (struct dirent* temp_set_type_dirent)
{
    if (temp_set_type_dirent->d_type == DT_REG)
        return FILES;
    else
        return LINKS;
}