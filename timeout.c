#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;

int read_timeout(char *s)
{
    char *p;
    int v;
    errno = 0;
    v = strtol(s, &p, 0);
    if (errno != 0 || *p != '\0') {
        fprintf(stderr, "Invalid value '%s'\n", s);
        exit(EXIT_FAILURE);
    }
    return v;
}

pid_t spawn(char **argv)
{
    pid_t p = fork();
    if (p == -1) {
        fprintf(stderr, "Error in fork()\n");
        exit(EXIT_FAILURE);
    }
    if (p != 0)
        return p;
    execve(argv[0], argv, environ);
    fprintf(stderr, "Error: cannot execute '%s'\n", argv[0]);
    exit(EXIT_FAILURE);
}

int ftimeout = 0;

void alarm_handler(int sig)
{
    (void) sig;
    ++ftimeout;
}

void start_alarm(int n)
{
    struct sigaction sa;

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        fprintf(stderr, "Error in sigaction()\n");
        exit(EXIT_FAILURE);
    }
    alarm(n);
}

void wait_alarm(pid_t pid)
{
    int status;
    pid_t p;
    p = wait(&status);
    if (p == -1 && errno != EINTR) {
        fprintf(stderr, "Error in wait()\n");
        exit(EXIT_FAILURE);
    }
    if (p == pid)
        return;
    if (ftimeout == 0 || p != -1 || errno != EINTR) {
        fprintf(stderr, "Unexpected internal error\n");
        exit(EXIT_FAILURE);
    }
    if (kill(pid, SIGTERM) == -1)
        fprintf(stderr, "Cannot send SIGTERM signal: already terminated?\n");
    else
        fprintf(stderr, "Killed process %llu\n", (unsigned long long) pid);
}

int main(int argc, char **argv)
{
    int timeout;
    pid_t pid;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <seconds> <program> [...]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    timeout = read_timeout(argv[1]);
    pid = spawn(argv+2);
    start_alarm(timeout);
    wait_alarm(pid);
    return EXIT_SUCCESS;
}
