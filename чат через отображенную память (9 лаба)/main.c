#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <errno.h>

struct user{
    char name[128];
    char last_message[256];
};

static int fd = 0;
struct user chat; 
int exit_x;

void *thread_func(void * arg);
bool uniqe(struct user * user1, struct user * user2);
static pthread_t * mutex;


int main() {
    system("clear");
    exit_x = 0;
    pthread_t * thread;
    struct user empty_user = {""};
    int shmid = shmget (200, sizeof (pthread_mutex_t *), IPC_CREAT | 0666 | IPC_EXCL);
    mutex = (pthread_mutex_t *) shmat (shmid, NULL, 0);
    fd = open("file", O_RDWR);
    printf("name: ");
    gets(chat.name);
    ftruncate (fd, sizeof(struct user));
    void * shared_memory = NULL;
    shared_memory = mmap(0, sizeof(struct user), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    pthread_create(&thread, NULL, thread_func, shared_memory);
    struct user temp_user_r;
    while(exit_x == 0) {
        memcpy(&temp_user_r, shared_memory, sizeof(struct user));
        if(uniqe((struct user *)&chat, (struct user *)&temp_user_r)) 
            continue;
        while(!strcmp(temp_user_r.last_message, "")){
            memcpy(&temp_user_r, shared_memory, sizeof(struct user));
        }
        printf("\n%s: %s\n", temp_user_r.name, temp_user_r.last_message);
        memcpy(shared_memory, &empty_user, sizeof(struct user));
    }
    shmdt (mutex);
}


void *thread_func(void * arg) {

    while(true) {
        pthread_mutex_lock (&mutex);
        fflush(stdin);
        gets(chat.last_message);
        if (strcmp (chat.last_message, "/exit") == 0)
        {
            exit_x = 1;
            break;
        }
        if(!strcmp(chat.last_message, "")) 
            continue;
        memcpy(arg, &chat, sizeof(struct user));
        pthread_mutex_unlock (&mutex);
    }

}


bool uniqe(struct user * user1, struct user * user2) {
    if(!strcmp(user2 -> name, "")) 
        return 1;
    if(strcmp(user1 -> name, user2 -> name)) 
        return 0;
    return 1;
}