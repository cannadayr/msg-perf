#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFSZ 65536
int main() {
    int sockets[2];

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) < 0) {
        fprintf(stderr, "couldn't create socket pair\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    int n_elems;
    n_elems = BUFSZ/sizeof(double);
    double buf[n_elems];
    int rval;

    // dummy data
    double data[n_elems];
    for(int i = 0; i < n_elems; i++) {
        data[i] = (double)i;
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "couldn't set signal handler\n");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid) {
        case -1:
            fprintf(stderr, "couldn't fork process\n");
            exit(EXIT_FAILURE);
        case 0:
            close(sockets[1]);
            if (write(sockets[0], data, sizeof(data)) < 0) {
                fprintf(stderr, "couldn't write msg to socket\n");
                exit(EXIT_FAILURE);
            }
            if (read(sockets[0], buf, sizeof(buf)) < 0) {
                fprintf(stderr, "couldn't read msg from socket\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "received data from server of size %d\n", sizeof(buf));
            close(sockets[0]);
            fprintf(stderr, "child proc exiting...\n");
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "parent proc. child proc PID %jd.\n", (intmax_t) pid);
            close(sockets[0]);
            if (rval = read(sockets[1], buf, sizeof(buf)) < 0) {
                fprintf(stderr, "couldn't read buffer\n");
                exit(EXIT_FAILURE);
            }
            if (rval == 0) {
                fprintf(stderr, "received data from client of size %d\n", sizeof(buf));
                if (write(sockets[1], &data, sizeof(data)) < 0) {
                    fprintf(stderr, "couldn't write to socket\n");
                    exit(EXIT_FAILURE);
                }
                close(sockets[1]);
                exit(EXIT_SUCCESS);
            }

            exit(EXIT_SUCCESS);
    }
    return 0;
}
