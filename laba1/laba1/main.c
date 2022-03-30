#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

struct Flags
{
    bool file;
    bool dir;
    bool link;
    bool sort;
    bool type;
    bool typeNotDir;
};

struct AllWalk
{
    char** list;
    int val;
};

void walkDir (const char* mainWalk, char* nameDir, struct AllWalk * temp, struct Flags * Flags);

bool findWalk (struct AllWalk * temp, char* walk);

void addWalk (char* name_dir, char* mainWalk, struct AllWalk  **temp);

int main(int argc, const char * argv[]) {
    DIR * file_path;
    struct dirent * file_dirent;
    char* mainWalk;
    if (argc == 1){
        mainWalk = malloc (500);
        getcwd(mainWalk, 500);
    }
    else{
        mainWalk = malloc(500);
        getcwd (mainWalk, 500);
        strcat (mainWalk, "/");
        strcat (mainWalk, *(argv + 1));
        strcat (mainWalk, "\0");
    }
    file_path = opendir(mainWalk);
    if (!file_path)
    {
        printf ("Не найдено\n");
        return 0;
    }
    struct Flags *Flags = (struct Flags*)malloc(sizeof(struct Flags));
    Flags->file = false;
    Flags->link = false;
    Flags->dir = false;
    Flags->sort = false;
    Flags->type = false;
    Flags->typeNotDir = false;
    char* str = malloc (strlen(mainWalk));
    strcpy(str, mainWalk);
    if (argc > 2)
    {
            if (strcmp(*(argv + 2), "-type") == 0){
                for (int j = 3; j < argc; j++)
                {
                    if (strcmp(*(argv + j), "f") == 0)
                        Flags->file = true;
                    if (strcmp(*(argv + j), "d") == 0)
                        Flags->dir = true;
                    if (strcmp(*(argv + j), "l") == 0)
                        Flags->link = true;
                    if (strcmp(*(argv + j), "s") == 0)
                        Flags->sort = true;
                }
                if (!Flags->file && !Flags->link && !Flags->dir && !Flags->sort)
                {
                    printf ("Неверный аргумент (-type [struct Flagsag])\n");
                    return 0;
                }
                Flags->type = true;
                if (!Flags->dir && !Flags->file && !Flags->link)
                    Flags->typeNotDir = true;
            }
    }
    struct AllWalk * temp = malloc (sizeof(struct AllWalk));
    temp->val = 1;
    temp->list = realloc (temp->list, temp->val * sizeof(char*));
    temp->list[temp->val - 1] = malloc (strlen(mainWalk));
    strcpy (temp->list[temp->val - 1], mainWalk);
    while ((file_dirent = readdir(file_path)))
        if (strcmp(file_dirent->d_name, ".") != 0 && strcmp(file_dirent->d_name, "..") != 0)
        {
            if (file_dirent->d_type == DT_LNK && ((Flags->type && Flags->link) || !Flags->type || Flags->typeNotDir))
                addWalk(file_dirent->d_name, mainWalk, &temp);
            if (file_dirent->d_type == DT_REG && ((Flags->type && Flags->file) || !Flags->type || Flags->typeNotDir))
                addWalk(file_dirent->d_name, mainWalk, &temp);
            if (file_dirent->d_type == DT_DIR)
            {
                if ((Flags->type && Flags->dir) || !Flags->type || Flags->typeNotDir)
                    addWalk(file_dirent->d_name, mainWalk, &temp);
                walkDir(mainWalk, file_dirent->d_name, temp, Flags);
            }
        }
    char* sw = malloc (1000);
    char* a, *b;
    a = malloc (1);
    b = malloc (1);
    if ((Flags->type && Flags->sort))
        for (int i = 0; i < temp->val; i++)
            for (int j = i; j < temp->val; j++)
                if (strcmp(temp->list[i], temp->list[j]) > 0)
                {
                    sw = temp->list[i];
                    temp->list[i] = temp->list[j];
                    temp->list[j] = sw;
                }
    int len = 0;
    for (int i = 0; i < strlen (mainWalk); i++)
        if (mainWalk[i] == '/')
            len = i;
    for (int i = 0; i < temp->val; i++){
        for (int k = len + 1; k < strlen (temp->list[i]); k++)
            printf ("%c", temp->list[i][k]);
        printf ("\n");
    }
    return 0;
}

void walkDir (const char* mainWalk, char* nameDir, struct AllWalk * temp, struct Flags * Flags)
{
    char* walk = malloc(strlen(mainWalk) + strlen(nameDir) + 2);
    strcpy (walk, mainWalk);
    strcat(walk, "/");
    strcat(walk, nameDir);
    strcat(walk, "\0");
    DIR * path;
    struct dirent * file_dir;
    if ((path = opendir(walk)))
        while ((file_dir = readdir(path)))
            if (strcmp(file_dir->d_name, ".") != 0 && strcmp(file_dir->d_name, "..") != 0)
            {
                if (file_dir->d_type == DT_LNK && ((Flags->type && Flags->link) || !Flags->type || Flags->typeNotDir))
                    addWalk(file_dir->d_name, walk, &temp);
                if (file_dir->d_type == DT_REG && ((Flags->type && Flags->file) || !Flags->type || Flags->typeNotDir))
                    addWalk(file_dir->d_name, walk, &temp);
                if (file_dir->d_type == DT_DIR)
                {
                    if ((Flags->type && Flags->dir) || !Flags->type || Flags->typeNotDir)
                        addWalk(file_dir->d_name, walk, &temp);
                    walkDir(walk, file_dir->d_name, temp, Flags);
                }
            }
}

bool findWalk (struct AllWalk * temp, char* walk)
{
    for (int i = 0; i < temp->val; i++)
    {
        if (strcmp(temp->list[i], walk) == 0)
            return false;
    }
    return true;
}

void addWalk (char* name_dir, char* mainWalk, struct AllWalk ** temp)
{
    char* str = malloc (1);
    str = realloc (str, strlen(mainWalk) + 2 + strlen(name_dir));
    strcpy (str, mainWalk);
    strcat(str, "/");
    strcat(str, name_dir);
    strcat(str, "\0");
    (*temp)->val++;
    (*temp)->list = realloc ((*temp)->list, (*temp)->val * sizeof(char*));
    (*temp)->list[(*temp)->val - 1] = malloc (strlen (str) + 1);
    if (findWalk((*temp), str))
    {
        strcpy ((*temp)->list[(*temp)->val - 1], str);
        strcat ((*temp)->list[(*temp)->val - 1], "\0");
    }
    return;
}
