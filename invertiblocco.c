#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void open_files(const char *infile, const char *outfile,
                int *in, int *out)
{

    *in = open(infile, O_RDONLY);
    if (*in == -1) {
        fprintf(stderr, "Cannot open file '%s'\n", infile);
        exit(EXIT_FAILURE);
    }
    *out = open(outfile, O_WRONLY|O_CREAT, 0644);
    if (*out == -1) {
        fprintf(stderr, "Cannot create file '%s'\n", outfile);
        exit(EXIT_FAILURE);
    }
}

unsigned long read_block_size(const char *str)
{
    unsigned long v;
    char *p;
    errno = 0;
    v = strtol(str, &p, 0);
    if (errno != 0 || *p != '\0') {
        fprintf(stderr, "Invalid block size '%s'\n", str);
        exit(EXIT_FAILURE);
    }
    return v;
}

int read_block(int in, char *buf, unsigned long size)
{
    unsigned long r;
    ssize_t v;

    r = 0;
    while (size > r) {
        v = read(in, buf, size-r);
        if (v == -1) {
            fprintf(stderr, "Error while reading file\n");
            exit(EXIT_FAILURE);
        }
        if (v == 0)
            return r;
        r += v;
        buf += v;
    }
    return size;
}

void invert_block(char *buf, unsigned long size)
{
    unsigned int i;

    for (i=0; i<size/2; ++i) {
        char tmp = buf[i];
        buf[i] = buf[size-1-i];
        buf[size-1-i] = tmp;
    }
}

void write_block(int out, char *buf, unsigned long size)
{
    ssize_t v;

    while (size > 0) {
        v = write(out, buf, size);
        if (v == -1) {
            fprintf(stderr, "Error while writing\n");
            exit(EXIT_FAILURE);
        }
        size -= v;
        buf += v;
    }
}

void copy_invert(int in, int out, unsigned long size)
{
    char *buf;
    unsigned long r;

    buf = alloca(size);

    for (;;) {
        r = read_block(in, buf, size);
        if (r == 0)
            break;
        invert_block(buf, r);
        write_block(out, buf, r);
    }
}

int main(int argc, char *argv[])
{
    int fd_in, fd_out;
    unsigned long n;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file input> <file output> <block size>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    open_files(argv[1], argv[2], &fd_in, &fd_out);
    n = read_block_size(argv[3]);
    copy_invert(fd_in, fd_out, n);
    exit(EXIT_SUCCESS);
}
