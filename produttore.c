#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

int main(void)
{
    struct circular_buffer *cb;

    cb = get_shared_memory();

    for (;;) {
        int c, nE;
        c = getchar();
        if (c == EOF)
            break;
        nE = (cb->E + 1) % cbuf_size;
        /* controllare se buffer pieno */
        while (nE == cb->S) {
            /* buffer pieno */
            usleep(100000);
        }
        cb->buf[cb->E] = c;
        cb->E = nE;
    }
    return EXIT_SUCCESS;
}
