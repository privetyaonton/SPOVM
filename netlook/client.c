 #include "FileSystem.h"

typedef struct
{
    char com [10];
    char arg [200];
    char sub_arg[200];
}command;

int recv_all (void* buffer, int len);
char* manual1 = {"cd [dir] - перейти к каталогу\nls - просмотреть объекты в каталоге\nhome - перейти в домашний каталог\n"};
char* manual2 = {"cat [filename] - вывести на экран содержимое файла\nexit - выйти\nhelp - вызвать мануал\n"};
char* manual3 = {"loadfile [filename] [walkload] - скачать файл\npwd - выводит действующий путь\n"};
int is_exit;
dir * home_dir;
void load_file (char* walk_load, file * load_file);
int tools_socket (int port, const char* addr);
int sockfd;


int main(int argc, char * argv[])
{
    is_exit = 0;
    if (argc != 2)
    {
        printf ("Неверное количество аргументов\n");
        return 0;
    }
    sockfd = tools_socket (8888, argv[1]);
    char input_message[1000];
    bzero (input_message, sizeof (input_message));
    char server_answer[1000];
    int length, length_walk, length_name;
    int signal_file;
    printf ("%s%s%s\n", manual1, manual2, manual3);
    while (1)
    {
        printf (">>>");
        gets (input_message);
        send (sockfd, input_message, 1000, 0);
        recv (sockfd, &signal_file, sizeof (int), 0);
        if (signal_file == 0)
        {
            recv_all (&length, sizeof (int));
            bzero (server_answer, sizeof (server_answer));
            recv_all (server_answer, length);
            printf ("%s\n", server_answer);
        }
        else if (signal_file == 1)
        {
            char name_file [MAX_SIZE_NAME_FILE];
            bzero (name_file, sizeof (name_file));
            recv_all (&length_name, sizeof (int));
            recv_all (name_file, length_name);
            recv_all (&length, sizeof (int));
            char recv_file [length];
            bzero (recv_file, sizeof (recv_file));
            recv_all (recv_file, length);
            char walk [MAX_LENGTH_NAME_WALK];
            bzero (walk, sizeof (walk));
            recv_all (&length_walk, sizeof (int));
            recv_all (walk, length_walk);
            file * temp = malloc (sizeof (file));
            init_file (temp, -1);
            memcpy (temp->data_file, recv_file, length);
            strcpy (temp->name_file, name_file);
            temp->size_data_file = length;
            load_file (walk, temp);
            free (temp);
        }
        else if (signal_file == 2) continue;
        else if (signal_file == 3) break;
    }
    close (sockfd);
    return 0;
}

int recv_all (void* buffer, int len)
{
    char* pbuf = (char*) buffer;
    int total = 0;
    while (len > 0)
    {
        int rval = recv(sockfd, pbuf, len, 0);
        if (rval < 0) {break;}
        if (rval == 0)
            break;
        pbuf += rval;
        len -= rval;
        total += rval;
    }
    return total;
}

void load_file (char* walk_load, file * load_file)
{
    FILE* file = fopen (load_file->name_file, "wb");
    if (file == NULL)
    {
        printf ("Неверный путь к файлу\n");
        return;
    }
    fwrite (load_file->data_file, 1, load_file->size_data_file, file);
    fclose (file);
    rename_name (load_file->name_file, calculating_space (load_file->name_file));
    move_file (walk_load, load_file->name_file);
}

int tools_socket (int port, const char* addr)
{
    struct sockaddr_in client_info;
    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf ("Ошибка при создании сокета\n");
        exit (0);
    }
    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    client_info.sin_addr.s_addr = inet_addr (addr);
    client_info.sin_family = AF_INET;
    client_info.sin_port = htons (port);
    if (connect (sock, (struct sockaddr *)&client_info, sizeof (client_info)) < 0)
    {
        printf ("Ошибка при соединении\nerrno = %d\n", errno);
        exit (0);
    }
    return sock;
}
