#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define cbuf_size 33

struct circular_buffer {
    pthread_mutex_t mtx;
    int S;
    int E;
    char buf[cbuf_size];
};

struct circular_buffer cb;

void *consume(void *p)
{
    struct circular_buffer *cb = p;

    for (;;) {
        if (pthread_mutex_lock(&cb->mtx) != 0) {
            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        while (cb->S == cb->E) {
           /* circular buffer empty */ ;
            if (pthread_mutex_unlock(&cb->mtx) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }
           usleep(100000);
           if (pthread_mutex_lock(&cb->mtx) != 0) {
               perror("pthread_mutex_lock");
               exit(EXIT_FAILURE);
           }
        }
        putchar(cb->buf[cb->S]);
        fflush(stdout);
        cb->S = (cb->S + 1) % cbuf_size;
        if (pthread_mutex_unlock(&cb->mtx) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }
        usleep(100000);
    }
}

void *produce(void *p)
{
    struct circular_buffer *cb = p;

    for (;;) {
        int c, nE;
        c = getchar();
        if (c == EOF)
            break;
        if (pthread_mutex_lock(&cb->mtx) != 0) {
            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        nE = (cb->E + 1) % cbuf_size;
        /* controllare se buffer pieno */
        while (nE == cb->S) {
            /* buffer pieno */
            if (pthread_mutex_unlock(&cb->mtx) != 0) {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }
            usleep(100000);
            if (pthread_mutex_lock(&cb->mtx) != 0) {
                perror("pthread_mutex_lock");
                exit(EXIT_FAILURE);
            }
            nE = (cb->E + 1) % cbuf_size;
        }
        cb->buf[cb->E] = c;
        cb->E = nE;
        if (pthread_mutex_unlock(&cb->mtx) != 0) {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(0);
}


int main(void)
{
    int s;
    pthread_t t;

    cb.S = cb.E = 0;
    if (pthread_mutex_init(&cb.mtx,NULL) != 0) {
        fprintf(stderr, "Error in pthread_mutex_init\n");
        exit(EXIT_FAILURE);
    }

    s = pthread_create(&t, NULL, consume, &cb);
    if (s != 0) {
        fprintf(stderr, "Error in pthread_create\n");
        exit(EXIT_FAILURE);
    }

    s = pthread_create(&t, NULL, consume, &cb);
    if (s != 0) {
        fprintf(stderr, "Error in pthread_create\n");
        exit(EXIT_FAILURE);
    }
    produce(&cb);
    return 0; /* never reached */
}
