#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

pthread_barrier_t brr;

void *job(void *p)
{
    long n = (long) p;
    unsigned long v;
    int s;

    for (;;) {
        for (v=0; v < 1000000; )
            v += n;

        s = pthread_barrier_wait(&brr);
        if (s != 0 && s != PTHREAD_BARRIER_SERIAL_THREAD) {
            fprintf(stderr, "pthread_barrier_wait() error\n");
            exit(EXIT_FAILURE);
        }
        if (s == PTHREAD_BARRIER_SERIAL_THREAD)
            printf("n=%ld, v=%lu\n", n, v);
    }
}

int main(void)
{
    int s;
    pthread_t t;

    if (pthread_barrier_init(&brr, NULL, 4) != 0) {
        fprintf(stderr, "pthread_barrier_init() error\n");
        exit(EXIT_FAILURE);
    }

    s = pthread_create(&t, NULL, job, (void *) 1);
    if (s != 0) {
        fprintf(stderr, "pthread_create() error\n");
        exit(EXIT_FAILURE);
    }
    s = pthread_create(&t, NULL, job, (void *) 2);
    if (s != 0) {
        fprintf(stderr, "pthread_create() error\n");
        exit(EXIT_FAILURE);
    }
    s = pthread_create(&t, NULL, job, (void *) 3);
    if (s != 0) {
        fprintf(stderr, "pthread_create() error\n");
        exit(EXIT_FAILURE);
    }
    s = pthread_create(&t, NULL, job, (void *) 4);
    if (s != 0) {
        fprintf(stderr, "pthread_create() error\n");
        exit(EXIT_FAILURE);
    }
    pthread_exit(0);
}
