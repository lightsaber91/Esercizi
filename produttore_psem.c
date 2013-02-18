#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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

    sid = shmget(key, sizeof(struct circular_buffer),
            IPC_CREAT | 0666);
    if (sid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    p = shmat(sid, NULL, 0);
    if (p == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    /* UNSAFE */
    if (sem_getvalue(&p->sem, &v) == -1 || v == 0) {
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
        int c, nE;
        c = getchar();
        if (c == EOF)
            break;
        if (sem_wait(&cb->sem) == -1) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
        nE = (cb->E + 1) % cbuf_size;
        /* controllare se buffer pieno */
        while (nE == cb->S) {
            /* buffer pieno */
            if (sem_post(&cb->sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }
            usleep(100000);
            if (sem_wait(&cb->sem) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }
            nE = (cb->E + 1) % cbuf_size;
        }
        cb->buf[cb->E] = c;
        cb->E = nE;
        if (sem_post(&cb->sem) == -1) {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}
