#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>

#define cbuf_size 33

struct circular_buffer {
    sem_t sem;
    int S;
    int E;
    char buf[cbuf_size];
};

struct circular_buffer *get_shared_memory(void)
{
    int sid, v;
    key_t key = ftok(".", 'b');
    struct circular_buffer *p;

    sid = shmget(key, sizeof(struct circular_buffer), IPC_CREAT | 0666);
    if (sid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    p = shmat(sid, NULL, 0);
    if (p == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    /* on creation the shared memory region is filled with zeros */
    if (sem_getvalue(&p->sem, &v) == -1 || v == 0) { /* UNSAFE */
        if (sem_init(&p->sem, 1, 1) == -1) {
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    return p;
}

int main(void)
{
    struct circular_buffer *cb;

    cb = get_shared_memory();

    for (;;) {
        if (sem_wait(&cb->sem) == -1) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }   
        while (cb->S == cb->E) {
           /* circular buffer empty */ ;
           if (sem_post(&cb->sem) == -1) {
               perror("sem_post");
               exit(EXIT_FAILURE);
           }   
           usleep(100000);
           if (sem_wait(&cb->sem) == -1) {
               perror("sem_wait");
               exit(EXIT_FAILURE);
           }   
        }
        putchar(cb->buf[cb->S]);
        fflush(stdout);
        cb->S = (cb->S + 1) % cbuf_size;
        if (sem_post(&cb->sem) == -1) {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }   
        usleep(100000);
    }
    return EXIT_SUCCESS;
}
