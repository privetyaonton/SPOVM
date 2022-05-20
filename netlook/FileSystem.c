#include "FileSystem.h"

void init_file (file* temp_file, int id)
{
    temp_file->id_file = id;
    temp_file->size_data_file = 0;
    temp_file->size_name_file = 0;
}

void init_dir (dir* temp_dir, int id)
{
    temp_dir->id_dirs = id;
    temp_dir->size_name_file = 0;
    temp_dir->number_entries_dirs = 0;
    temp_dir->number_entries_files = 0;
}

file* find_file (array_files* files, int id)
{
    for (int i = 0; i < files->size; i++)
        if (files->array[i].id_file == id)
            return &files->array[i];
}

dir* find_dir (array_dirs* dirs, int id)
{
    for (int i = 0; i < dirs->size; i++)
        if (dirs->array[i].id_dirs == id)
            return &dirs->array[i];
}

void convert_file_to_data (const char* walk_file, const char* name_file, file * this_file, const char* name_file_mv)
{
    char work_walk[500];
    getcwd (work_walk, 500);
    char temp_walk [MAX_LENGTH_NAME_WALK];
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
    char data [size_file];
    this_file->size_data_file = size_file;
    fread (data, 1, size_file, file);
    fclose (file);
    memcpy (this_file->data_file, data, size_file);
    strcpy (this_file->name_file, name_file);
    move_file (walk_file, name_file_mv);
}

int move_file (const char* walk, const char* name)
{
    char mv [MAX_LENGTH_NAME_WALK];
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

void rename_name (char* name_file, int number_spaces)
{
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
}