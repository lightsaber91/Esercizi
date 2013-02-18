#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int bsem_init_busy(int bsemID)
{
    union semun arg;

    arg.val = 0; /* in use */
    return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_init_free(int bsemID)
{
    union semun arg;

    arg.val = 1; /* free */
    return semctl(bsemID, 0, SETVAL, arg);
}

int bsem_get(int bsemID)
{
    struct sembuf sops;

    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    while (semop(bsemID, &sops, 1) == -1)
        if (errno != EINTR) /* ! interrupted by a signal */
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
    int bsemID;
    struct circular_buffer *cb;

    cb = get_shared_memory();
    bsemID = get_ipc_semaphore();

    for (;;) {
        if (bsem_get(bsemID) == -1) {
            perror("bsem_get");
            exit(EXIT_FAILURE);
        }
        while (cb->S == cb->E) {
           /* circular buffer empty */ ;
           if (bsem_put(bsemID) == -1) {
               perror("bsem_put");
               exit(EXIT_FAILURE);
           }
           usleep(100000);
           if (bsem_get(bsemID) == -1) {
               perror("bsem_get");
               exit(EXIT_FAILURE);
           }
        }
        putchar(cb->buf[cb->S]);
        fflush(stdout);
        cb->S = (cb->S + 1) % cbuf_size;
        if (bsem_put(bsemID) == -1) {
            perror("bsem_put");
            exit(EXIT_FAILURE);
        }
        usleep(100000);
    }
    return EXIT_SUCCESS;
}
