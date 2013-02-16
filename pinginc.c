#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

long read_num(char *str)
{
    long v;
    char *p;

    errno = 0;
    v = strtol(str, &p, 0);
    if (errno != 0 || *p != '\0') {
        fprintf(stderr, "Invalid value '%s'\n", str);
        exit(EXIT_FAILURE);
    }
    return v;
}

int get_msgqueue(void)
{
    key_t key;
    int id;

    key = ftok(".", 'a');
    if (key == -1) {
        fprintf(stderr, "Error in ftok\n");
        exit(EXIT_FAILURE);
    }
    id = msgget(key, IPC_CREAT | 0666);
    if (id == -1) {
        fprintf(stderr, "Error in msgget\n");
        exit(EXIT_FAILURE);
    }
    return id;
} 
    
struct msg_num {
    long mtype;
    long value;
};

long receive_number(int qid)
{
    int rc;
    struct msg_num mn;
    const int msg_size = sizeof(mn)-sizeof(long);

    /* Attenzione! msgrcv() si aspetta di avere la dimensione
     * massima del messaggio *senza* considerare il primo campo di
     * tipo long (MC120713) */

    rc = msgrcv(qid, &mn, msg_size, 0, 0);
    if (rc != msg_size) {
        fprintf(stderr, "Error in msgrcv\n");
        exit(EXIT_FAILURE);
    }
    return mn.value;
}

void send_number(int qid, long n)
{
    int rc;
    struct msg_num mn;
    const int msg_size = sizeof(mn)-sizeof(long);

    mn.mtype = 1;
    mn.value = n;

    /* Attenzione! msgsnd() si aspetta di avere la dimensione
     * del messaggio *senza* considerare il primo campo di
     * tipo long (MC120713) */

    rc = msgsnd(qid, &mn, msg_size, 0);
    if (rc == -1) {
        fprintf(stderr, "Error in msgsnd\n");
        exit(EXIT_FAILURE);
    }
}

void wait_and_send(int qid)
{
    long n;
    n = receive_number(qid);
    printf("%llu:%ld\n", (unsigned long long)getpid(),
        n);
    send_number(qid, n+1);
}
    

int main(int argc, char *argv[])
{
    int qid;
    qid = get_msgqueue();

    if (argc > 1) {
        long n = read_num(argv[1]);
        send_number(qid, n);
    }

    for (;;) 
        wait_and_send(qid);
}
