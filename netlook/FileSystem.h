#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#define MAX_SIZE_FILE 1073741824 // max = 1GB
#define MAX_SIZE_NAME_FILE 200
#define MAX_NUMBERS_ENTRIES_OBJECTS 64
#define MAX_NUMBERS_ENTRIES_DIRS 64
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
#include <pthread.h>

struct file
{
    int type;
    char data_file[MAX_SIZE_FILE];
    int size_data;
    char name_file[MAX_SIZE_NAME_FILE];
    int size_name_file;
};

struct my_dir
{
    int size_dirs;
    struct file * objects;
    int size_objects;
    char name_dir[MAX_SIZE_NAME_FILE];
    int size_name_dir;
    struct my_dir * dirs;
};

void convert_file_to_data (const char* walk_file, const char* name_file, struct file * this_file, const char * name_file_mv);
int move_file (const char* walk, const char* name);
int calculating_space (const char* str);


#endif