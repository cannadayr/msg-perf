#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFSZ 1024
int main() {
    int sockets[2];

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) < 0) {
        fprintf(stderr, "couldn't create socket pair\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    char buf[BUFSZ];
    int rval;

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
            fprintf(stderr, "child proc exiting...\n");
            close(sockets[1]);
            if (write(sockets[0], "PING", sizeof("PING")) < 0) {
                fprintf(stderr, "couldn't write msg to socket\n");
                exit(EXIT_FAILURE);
            }
            if (read(sockets[0], buf, BUFSZ) < 0) {
                fprintf(stderr, "couldn't read msg from socket\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "-->%s\n", buf);
            close(sockets[0]);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "parent proc. child proc PID %jd.\n", (intmax_t) pid);
            close(sockets[0]);
            if (rval = read(sockets[1], buf, sizeof(buf)) < 0) {
                fprintf(stderr, "couldn't read buffer\n");
                exit(EXIT_FAILURE);
            }
            if (rval == 0) {
                // Ensure buffer is 0-terminated.
                buf[sizeof(buf) - 1] = 0;
                fprintf(stderr, "-->%s\n", buf);
                if (write(sockets[1], "PONG", sizeof ("PONG")) < 0) {
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
