#ifndef FILESYSTEM_H
#define _CRT_SECURE_NO_WARNINGS 
#define _CRT_NONSTDC_NO_WARNINGS
#define FILESYSTEM_H
#define MAX_SIZE_FILE 1073741824 // max = 1GB
#define MAX_SIZE_NAME_FILE 200
#define MAX_NUMBERS_ENTRIES_OBJECTS 64
#define MAX_NUMBERS_ENTRIES_DIRS 64
#define MAX_SIZE_DIRS 16
#define MAX_SIZE_FILES 64
#define MAX_LENGTH_NAME_WALK 2000
#define FILES 0
#define LINKS 1
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

typedef struct
{
    int id_file;
    char name_file[MAX_SIZE_NAME_FILE];
    char data_file[MAX_SIZE_FILE];
    int size_name_file;
    int size_data_file;
    const char walk_to_file [MAX_LENGTH_NAME_WALK];
    const char walk_to_file_pc [MAX_LENGTH_NAME_WALK];
}file;

typedef struct 
{
    int id_dirs;
    int dirs [MAX_NUMBERS_ENTRIES_DIRS];
    int files[MAX_NUMBERS_ENTRIES_OBJECTS];
    char name_dir [MAX_SIZE_NAME_FILE];
    int size_name_file;
    int number_entries_dirs;
    int number_entries_files;
    const char walk_to_file [MAX_LENGTH_NAME_WALK];
}dir;

typedef struct 
{
    file* array;
    int size;
}array_files;

typedef struct 
{
    dir* array;
    int size;
}array_dirs;

void init_file (file* temp_file, int id);
void init_dir (dir* temp_dir, int id);
file* find_file (array_files * files, int id);
dir* find_dir (array_dirs* dirs, int id);

void convert_file_to_data (const char* walk_file, const char* name_file, file * this_file, const char * name_file_mv);
int move_file (const char* walk, const char* name);
int calculating_space (const char* str);
void rename_name (char* name_file, int number_spaces);


#endif