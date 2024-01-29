#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFSZ 65536

typedef struct {
    int sender;
    int receiver;
} socket_pair;

socket_pair create_dgram_pair() {
    int sockets[2];
    socket_pair spair;

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) < 0) {
        fprintf(stderr, "couldn't create socket pair\n");
        exit(EXIT_FAILURE);
    }

    spair.sender = sockets[0];
    spair.receiver = sockets[1];
    return spair;
}

double* create_dummy_data(int n_elems) {
    double* data = malloc(n_elems*sizeof(double));
    for(int i = 0; i < n_elems; i++) {
        data[i] = (double)i;
    }
    return data;
}

int main() {

    pid_t pid;
    int n_elems;
    n_elems = BUFSZ/sizeof(double);
    double buf[n_elems];
    int rval;

    socket_pair spair;
    spair = create_dgram_pair();

    double* data;
    data = create_dummy_data(n_elems);

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
            close(spair.receiver);
            if (write(spair.sender, data, sizeof(data)) < 0) {
                fprintf(stderr, "couldn't write msg to socket\n");
                exit(EXIT_FAILURE);
            }
            if (read(spair.sender, buf, sizeof(buf)) < 0) {
                fprintf(stderr, "couldn't read msg from socket\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "received data from server of size %d\n", sizeof(buf));
            close(spair.sender);
            fprintf(stderr, "child proc exiting...\n");
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "parent proc. child proc PID %jd.\n", (intmax_t) pid);
            close(spair.sender);
            if (rval = read(spair.receiver, buf, sizeof(buf)) < 0) {
                fprintf(stderr, "couldn't read buffer\n");
                exit(EXIT_FAILURE);
            }
            if (rval == 0) {
                fprintf(stderr, "received data from client of size %d\n", sizeof(buf));
                if (write(spair.receiver, &data, sizeof(data)) < 0) {
                    fprintf(stderr, "couldn't write to socket\n");
                    exit(EXIT_FAILURE);
                }
                close(spair.receiver);
                exit(EXIT_SUCCESS);
            }

            exit(EXIT_SUCCESS);
    }
    return 0;
}
