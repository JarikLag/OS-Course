#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <sys/un.h>

const int BUFFER_SIZE = 1024;
const int PIPE_SIZE = 2;
const char* SOCKET_NAME = "./socket";

int open_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Can't create a socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    return sock;
}

void binding(int listener) {
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCKET_NAME, strlen(SOCKET_NAME) + 1);
    if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cerr << "Error while binding: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int accepting(int listener) {
    int sock = accept(listener, nullptr, nullptr);
    if (sock < 0) {
        std::cerr << "Can't accept: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    return sock;
}

void create_pipe(int pipe_fd[PIPE_SIZE]) {
    if (pipe(pipe_fd) < 0) {
        std::cerr << "Can't create pipe: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    int tries = 20;
    unlink(SOCKET_NAME);
    int listener = open_socket();
    binding(listener);
    if (listen(listener, 1) < 0) {
        std::cerr << "Error while listening: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < tries; ++i) {
        int sock = accepting(listener);
        int pipe_fd[PIPE_SIZE];
        create_pipe(pipe_fd);

        struct msghdr msg = {nullptr};
        struct cmsghdr *cmsg;
        char io_buffer[1];
        struct iovec io = {
                .iov_base = io_buffer,
                .iov_len = sizeof(io_buffer)
        };
        union {
            char buf[CMSG_SPACE(sizeof(pipe_fd[1]))];
            struct cmsghdr align;
        } u;
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;
        msg.msg_control = u.buf;
        msg.msg_controllen = sizeof(u.buf);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(pipe_fd[1]));
        memcpy(CMSG_DATA(cmsg), &pipe_fd[1], sizeof(pipe_fd[1]));

        char buffer[CMSG_SPACE(sizeof pipe_fd[1])];

        if(sendmsg(sock, &msg, 0) < 0) {
            std::cerr << "Error while sending message: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        if (close(pipe_fd[1]) < 0) {
            std::cerr << "Can't close pipe: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        while (read(pipe_fd[0], &buffer, 1) > 0) {
            write(STDOUT_FILENO, &buffer, 1);
        }

        if (close(pipe_fd[0]) < 0) {
            std::cerr << "Can't close pipe: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        if (close(sock) < 0) {
            std::cerr << "Can't close sock: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    if (close(listener) < 0) {
        std::cerr << "Can't close socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}