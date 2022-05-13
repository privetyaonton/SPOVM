#include "FileSystem.h"

void convert_file_to_data (const char* walk_file, const char* name_file, struct file * this_file, const char* name_file_mv)
{
    char* work_walk = malloc (500);
    getcwd (work_walk, 500);
    char* temp_walk = malloc (strlen (walk_file) + strlen (name_file) + 1);
    strcpy (temp_walk, walk_file);
    strcat (temp_walk, "/");
    strcat (temp_walk, name_file_mv);
    if (move_file (work_walk, temp_walk) == -1)
        return;
    FILE* file = fopen (name_file, "rb");
    struct stat info_file;
    stat (name_file, &info_file);
    int size_file = info_file.st_size;
    if (size_file > MAX_SIZE_FILE)
    {
        printf ("В вашей папке есть очень большой объект - %s/%s", walk_file, name_file);
        return;
    }
    char* data = malloc (size_file);
    this_file->size_data = size_file;
    fread (data, 1, size_file, file);
    fclose (file);
    strcpy (this_file->data_file, data);
    strcpy (this_file->name_file, name_file);
    move_file (walk_file, name_file_mv);
}

int move_file (const char* walk, const char* name)
{
    char* mv = malloc (3 + strlen (walk) + strlen (name) + 1);
    strcpy (mv, "mv ");
    strcat (mv, name);
    strcat (mv, " ");
    strcat (mv, walk);
    int er = system (mv);
    if (er != 0)
    {
        system ("clear");
        printf ("Такого файла не было найдено или неверный путь\n");
        return -1;
    }
    return 0;
}

int calculating_space (const char* str)
{
    int size = strlen (str);
    int count = 0;
    for (int i = 0; i < size; i++)
        if (str[i] == ' ')
            count++;
    return count;
}