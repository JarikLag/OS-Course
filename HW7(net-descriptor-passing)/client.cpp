#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <ctime>
#include <sys/un.h>

const char* SOCKET_NAME = "./socket";

int open_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Can't create a socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    return sock;
}

void connecting(int sock) {
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCKET_NAME, strlen(SOCKET_NAME) + 1);
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cerr << "Can't connect client socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    std::srand(std::time(nullptr));
    int sock = open_socket();
    connecting(sock);

    struct msghdr msg = {nullptr};
    char io_buffer[1];
    struct iovec io = {
            .iov_base = io_buffer,
            .iov_len = sizeof(io_buffer)
    };
    union {
        char buf[CMSG_SPACE(sizeof(int))];
        struct cmsghdr align;
    } u;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);

    ssize_t read_bytes = recvmsg(sock, &msg, 0);
    if (read_bytes <= 0) {
        std::cerr << "Receive message failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

    std::string message = "Here is your random number: " + std::to_string(rand()) + "\n";

    write(fd, message.data(), message.size());

    if (close(fd) < 0) {
        std::cerr << "Can't close fd: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (close(sock) < 0) {
        std::cerr << "Can't close socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}