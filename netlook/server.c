#include "FileSystem.h"

typedef struct
{
    char com [100];
    char arg [200];
    char sub_arg[200];
}command;

char* manual1 = {"cd [dir] - перейти к каталогу\nls - просмотреть объекты в каталоге\nhome - перейти в домашний каталог\n"};
char* manual2 = {"cat [filename] - вывести на экран содержимое файла\nexit - выйти\nhelp - вызвать мануал\n"};
char* manual3 = {"loadfile [filename] [walkload] - скачать файл\npwd - выводит действующий путь в общей папке\n"};

int size_list_command = 8;
int send_all(const void * buffer, int len);
int tools_socket (int port);
int collect_dir (char* walk_dir, dir * home_dir);
static int id_count = 1;
int is_exit;
array_dirs * dirs;
array_files * files;
int sockfd;
command * parse_command (const char * message_client);
dir* home_dir;
int execute_command (command * pack_command, dir * this_dir);
void send_message (char *message, int signal, int signal_skip);
char* get_string_from_int (int value);


int main(int argc, char* argv[])
{
    is_exit = 0;
    if (argc != 2)
    {
        printf ("Неверное количество аргументов: ./server <walk_dir>\n");
        exit (0);
    }
    printf ("Ожидайте...\n");
    dirs = malloc (sizeof (array_dirs));
    files = malloc (sizeof (array_files));
    home_dir = malloc (sizeof (dir));
    init_dir (home_dir, 0);
    strcpy (home_dir->walk_to_file, "home/");
    if (collect_dir (argv[1], home_dir) < 0) {exit (0);}
    strcpy (home_dir->name_dir, "home");
    dirs->size++;
    dirs->array = (dir*) realloc (dirs->array, dirs->size * sizeof (dir));
    memcpy (&dirs->array[dirs->size - 1], home_dir, sizeof (dir));
    printf ("Сервер готов к соединению\n");
    sockfd = tools_socket(8888);
    printf ("Соединение установлено\n");
    char client_message[1000];
    int id_this_dir = 0;
    while (!is_exit)
    {
        recv (sockfd, client_message, 1000, 0);
        command * this_pack = malloc (sizeof (command));
        this_pack = parse_command (client_message);
        printf ("команда - %s аргумент - %s аргумент - %s\n", this_pack->com, this_pack->arg, this_pack->sub_arg);
        if (this_pack == NULL) continue;
        id_this_dir = execute_command(this_pack, find_dir (dirs, id_this_dir));
        free (this_pack);
    }
    close (sockfd);
    free (dirs);
    free (files);
    free (home_dir);
    return 0;
}

int collect_dir (char* walk_dir, dir* home_dir)
{
    struct dirent* dirent_file;
    DIR* file_path = opendir (walk_dir);
    char name_file [MAX_SIZE_NAME_FILE];
    if (file_path == NULL)
    {
        printf ("Такой директории не существует\n");
        return -1;
    }
    while ((dirent_file = readdir (file_path)))
    {
        if (dirent_file->d_name[0] != '.')
        {
            int number_spaces = calculating_space (dirent_file->d_name);
            bzero (name_file, sizeof (name_file));
            strcpy (name_file, dirent_file->d_name);
            rename_name (name_file, number_spaces);
            if (strlen(name_file) > MAX_SIZE_NAME_FILE)
            {
                printf ("Слишком большое название у файла: %s/%s", walk_dir, dirent_file->d_name);
                break;
            }
            if (dirent_file->d_type == DT_REG)
            {
                if (home_dir->number_entries_files == MAX_NUMBERS_ENTRIES_OBJECTS)
                {
                    printf ("Очень много вложенных файлов в папке: %s/%s", walk_dir, dirent_file->d_name);
                    return -1;
                }
                files->size++;
                files->array = (file*) realloc (files->array, files->size * sizeof (file));
                files->array[files->size - 1].size_name_file = strlen (dirent_file->d_name);
                strcpy (files->array[files->size - 1].name_file, dirent_file->d_name);
                init_file (&files->array[files->size - 1], id_count);
                home_dir->files[home_dir->number_entries_files] = id_count;
                home_dir->number_entries_files++;
                id_count++;
                strcpy (files->array[files->size - 1].walk_to_file, home_dir->walk_to_file);
                strcpy (files->array[files->size - 1].walk_to_file_pc, walk_dir);
                convert_file_to_data (walk_dir, dirent_file->d_name, &files->array[files->size - 1], name_file);
            }
            if (dirent_file->d_type == DT_DIR)
            {
                if (home_dir->number_entries_dirs == MAX_NUMBERS_ENTRIES_DIRS)
                {
                    printf ("Очень много вложtнных папок в папке %s/%s", walk_dir, dirent_file->d_name);
                    return -1;
                }
                dirs->size++;
                dirs->array = (dir*) realloc (dirs->array, dirs->size * sizeof (dir));
                init_dir (&dirs->array[dirs->size - 1], id_count);
                home_dir->dirs[home_dir->number_entries_dirs] = id_count;
                home_dir->number_entries_dirs++;
                id_count++;
                char walk [MAX_LENGTH_NAME_WALK];
                strcpy (dirs->array[dirs->size - 1].walk_to_file, home_dir->walk_to_file);
                strcpy (walk, walk_dir);
                strcat (walk, "/");
                strcat (walk, name_file);
                strcpy (dirs->array[dirs->size - 1].name_dir, dirent_file->d_name);
                dirs->array[dirs->size - 1].size_name_file = strlen (dirent_file->d_name);
                strcat (dirs->array[dirs->size - 1].walk_to_file, dirs->array[dirs->size - 1].name_dir);
                if (collect_dir(walk, &dirs->array[dirs->size - 1]) < 0)
                    return -1;
            }
        }
    }
}

int send_all(const void * buffer, int len)
{
    const char *pbuf = (const char *) buffer;
    while (len > 0)
    {
        int sent = send(sockfd, pbuf, len, 0);
        if (sent < 1) {break;}
        pbuf += sent;
        len -= sent;
    }
    return 0;
}

int tools_socket (int port)
{
    int server_sock_fd, client_sock_fd;
    struct sockaddr_in server_buffer_info;
    server_sock_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (server_sock_fd < 0)
    {
        printf ("Ошибка при создании сокета - Error %d", errno);
        exit (0);
    }
    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    bzero(&server_buffer_info, sizeof(struct sockaddr_in));
    server_buffer_info.sin_addr.s_addr = htonl (INADDR_ANY);
    server_buffer_info.sin_family = AF_INET;
    server_buffer_info.sin_port = htons (port);
    if (bind (server_sock_fd, (struct sockaddr *)&server_buffer_info, sizeof (server_buffer_info)) < 0)
    {
        printf ("Вы очень часто подключаетесь к адресу, подождите немного...\n");
        exit (0);
    }
    if (listen (server_sock_fd, 50) < 0)
    {
        printf ("Ошибка при прослушивании\nerrno = %d\n", errno);
        exit (0);
    }
    client_sock_fd = accept (server_sock_fd, NULL, NULL);
    return client_sock_fd;
}

command * parse_command (const char* message_client)
{
    char input_str[1000];
    char first_half[100];
    char second_half[100];
    char three_half[100];
    int count = 0;
    int end_count = 0;
    int open_index [5];
    int end_index [5];
    dir * dir = home_dir;
    command * pack = malloc (sizeof (command));
    for (int i = 0; i < strlen (message_client); i++)
    {
        if (message_client[i] == '[')
        {
            open_index[count] = i;
            count++;
        }
        if (message_client[i] == ']')
        {
            end_index[end_count] = i;
            end_count++;
        }
    }
    if (count > 2 || end_count > 2)
    {
        send_message ("Неверное количество аргументов\n", 0, 0);
        return NULL;
    }
    if (count != end_count)
    {
        send_message ("Проверьте открытие и закрытие скобок\n", 0, 0);
        return NULL;
    }
    if (count == 0) memcpy (pack->com, message_client, strlen (message_client));
    if (count > 0)
    {
        memcpy (pack->com, message_client, open_index[0] - 1);
        memcpy (pack->arg, message_client + open_index[0] + 1, end_index[0] - open_index[0] - 1);
        if (count > 1) memcpy (pack->sub_arg, message_client + open_index[1] + 1, end_index[1] - open_index[1] - 1);
    }
    return pack;
}

int execute_command (command * pack_command, dir * this_dir)
{
    if (strcmp (pack_command->com, "cd") == 0)
    {
        for (int i = 0; i < this_dir->number_entries_dirs; i++)
        {
            dir * temp = find_dir (dirs, this_dir->dirs[i]);
            if (atoi (pack_command->arg) == temp->id_dirs)
            {
                send_message ("\n", 2, 0);
                return this_dir->dirs[i];
            }
        }
        send_message ("Такой директории не было найдено\n", 0, 0);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "ls") == 0)
    {
        char mes[2000];
        char temp[MAX_SIZE_NAME_FILE];
        for (int i = 0; i < this_dir->number_entries_dirs; i++)
        {
            strcpy (temp, find_dir (dirs, this_dir->dirs[i])->name_dir);
            if (i == 0)
                strcpy (mes, temp);
            else
                strcat (mes, temp);
            strcat (mes, " id = ");
            strcat (mes, get_string_from_int (find_dir(dirs, this_dir->dirs[i])->id_dirs));
            strcat (mes, " DIR\n");
        }
        for (int i = 0; i < this_dir->number_entries_files; i++)
        {
            strcpy (temp, find_file (files, this_dir->files[i])->name_file);
            if (this_dir->number_entries_dirs == 0 && i == 0)
                strcpy (mes, temp);
            else
                strcat (mes, temp);
            strcat (mes, " id = ");
            strcat (mes, get_string_from_int (find_file(files, this_dir->files[i])->id_file));
            strcat (mes, " FILE\n");
        }
        send_message (mes, 0, 0);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "home") == 0)
    {
        send_message ("\n", 2, 0);
        return 0;
    }
    else if (strcmp (pack_command->com, "cat") == 0)
    {
        int number_spaces = calculating_space (pack_command->arg);
        char name_file[MAX_SIZE_NAME_FILE];
        strcpy (name_file, pack_command->arg);
        if (number_spaces != 0) rename_name (name_file, number_spaces);
        char main_walk[1000];
        char temp_name[MAX_SIZE_NAME_FILE];
        for (int i = 0; i < this_dir->number_entries_files; i++)
        {
            file * temp = find_file (files, this_dir->files[i]);
            if (atoi (pack_command->arg) == temp->id_file)
            {
                strcpy (main_walk, temp->walk_to_file_pc);
                break;
            }
        }
        file * ttemp = malloc (sizeof (file));
        init_file (ttemp, -1);
        convert_file_to_data (main_walk, pack_command->arg, ttemp, name_file);
        send_message (ttemp->data_file, 0, 0);
        free (ttemp);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "exit") == 0)
    {
        is_exit = 1;
        send_message ("\n", 3, 0);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "loadfile") == 0)
    {
        for (int i = 0; i < this_dir->number_entries_files; i++)
        {
            file * temp = malloc (sizeof (file));
            memcpy (temp, find_file (files, this_dir->files[i]), sizeof (file));
            if (atoi (pack_command->arg) == temp->id_file)
            {
                send_message (temp->name_file, 1, 0);
                send_all (&temp->size_data_file, sizeof (int));
                send_all (temp->data_file, temp->size_data_file);
                send_message (pack_command->sub_arg, 0, 1);
                return this_dir->id_dirs;
            }
            free (temp);
        }
        send_message ("Такого файла нет в этом каталоге\n", 0, 0);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "pwd") == 0)
    {
        send_message (this_dir->walk_to_file, 0, 0);
        return this_dir->id_dirs;
    }
    else if (strcmp (pack_command->com, "help") == 0)
    {
        char message [500];
        strcpy (message, manual1);
        strcat (message, manual2);
        strcat (message, manual3);
        send_message (message, 0, 0);
        return this_dir->id_dirs;
    }
    else {
        send_message ("Неверная команда", 0, 0);
        return this_dir->id_dirs;
    }
    return -1;
}

void send_message (char *message, int signal, int signal_skip)
{
    int length = strlen (message);
    if (!signal_skip)
        send (sockfd, &signal, sizeof (int), 0);
    if (signal > 1) return;
    send_all (&length, sizeof (int));
    send_all (message, length);
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
    char result [range];
    for (int i = range; i > 0; i--)
    {
        result[range - i] = (char)(48 + (int)((value / pow (10, i - 1))) % 10);
    }
    result[range] = '\0';
    return result;
}

