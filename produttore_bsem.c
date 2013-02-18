#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int bsem_init_free(int bsemID)
{
    union semun arg;
    arg.val = 1;
    return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_init_busy(int bsemID)
{
    union semun arg;
    arg.val = 0;
    return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_get(int bsemID)
{
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    while (semop(bsemID, &sops, 1) == -1)
        if (errno != EINTR)
            return -1;
    return 0;
}

int bsem_put(int bsemID)
{
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(bsemID, &sops, 1);
}

#define cbuf_size 33

struct circular_buffer {
    int S;
    int E;
    char buf[cbuf_size];
};

struct circular_buffer *get_shared_memory(void)
{
    int sid;
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
    return p;
}

int get_ipc_semaphore(void)
{
    int sid;
    key_t key = ftok(".", 'b');
    sid = semget(key, 1, IPC_CREAT | 0666);
    if (sid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    bsem_init_free(sid); /* UNSAFE */
    return sid;
}

int main(void)
{
    struct circular_buffer *cb;
    int bsemID;

    cb = get_shared_memory();
    bsemID = get_ipc_semaphore();

    for (;;) {
        int c, nE;
        c = getchar();
        if (c == EOF)
            break;
        if (bsem_get(bsemID) == -1) {
            perror("bsem_get");
            exit(EXIT_FAILURE);
        }
        nE = (cb->E + 1) % cbuf_size;
        /* controllare se buffer pieno */
        while (nE == cb->S) {
            /* buffer pieno */
            if (bsem_put(bsemID) == -1) {
                perror("bsem_put");
                exit(EXIT_FAILURE);
            }
            usleep(100000);
            if (bsem_get(bsemID) == -1) {
                perror("bsem_get");
                exit(EXIT_FAILURE);
            }
            nE = (cb->E + 1) % cbuf_size;
        }
        cb->buf[cb->E] = c;
        cb->E = nE;
        if (bsem_put(bsemID) == -1) {
            perror("bsem_put");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}
