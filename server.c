#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#define MAX 80

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};

int init_socket(int port) {
    //open socket, return socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        _exit(ERR_SOCKET);
    }
 
    //set socket option
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));
    if (server_socket < 0) {
        perror("Fail: set socket options");
        _exit(ERR_SETSOCKETOPT);
    }

    //set socket address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_BIND);
    }

    //listen mode start
    if (listen(server_socket, 5) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_LISTEN);
    }
    return server_socket;
}

struct sockaddr_in *clients_init(int port, int count_of_users, int **sockets){
    struct sockaddr_in *server_address = NULL;
    int errno;
    for (int i = 0; i < count_of_users; i++) {
        server_address = (struct sockaddr_in *)realloc(server_address, (i + 1) * sizeof(struct sockaddr_in));
        server_address[i].sin_family = AF_INET;
        server_address[i].sin_addr.s_addr = INADDR_ANY;
        server_address[i].sin_port = htons(port + i);
        errno = bind(*sockets[i], (struct sockaddr *) &server_address[i], sizeof(server_address[i]));
        if (errno) {
            err(1, NULL);
        }
    }
    return server_address;
}

int *client_sockets_init(int count_of_users)
{
    int *server_sockets = NULL;
    for(int i = 0; i < count_of_users; i++) {
        server_sockets = (int *)realloc(server_sockets, (i + 1) * sizeof(int));
        server_sockets[i] = socket(PF_INET, SOCK_STREAM, 0);
    }
    return server_sockets;
}

void receiver(unsigned int fd_in, unsigned int fd_out){
    unsigned char id, size;
    char buf[256];
    int ret;
    while(1){
        ret = read(fd_in,  &size, 1);
        if(ret < 0){
            exit(1);
        }
        read(fd_in, buf, size);
        write(fd_out, &id, 1);
        write(fd_out, &size, 1);
        write(fd_out, buf, size);
    }
}

char *read_fd(ssize_t fd, char *out, int size)
{
    if (read(fd, out, size) < 0) {
        err(1, NULL);
    }
    return out;
}

void write_fd(char *str, ssize_t fd)
{
    if (write(fd, str, sizeof(str)) < 0) {
        err(1, NULL);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port>");
        puts("Example:");
        puts("./server 8000");
        return ERR_INCORRECT_ARGS;
    }
    int i = 0, port = atoi(argv[1]), users = atoi(argv[2]);
    int server_socket = init_socket(port);
    int *client_socket = client_sockets_init(users);
    struct sockaddr_in *clients = clients_init(port, users, &client_socket);
    for (i = 0; i < users; i++) {
        puts("Wait for connection");
        struct sockaddr_in client_address;
        socklen_t size;
        client_socket[i] = accept(server_socket, 
                                    (struct sockaddr *) &client_address,
                                    &size);
        printf("connected: %s %d\n", inet_ntoa(client_address.sin_addr),
                                        ntohs(client_address.sin_port));
    }
    int pd[2];
    int pid[105];
    pipe(pd);
    for (i = 0; i < users; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            close(pd[0]);
            receiver(client_socket[i], pd[1]);
            close(pd[1]);
            return 0;
        }
    }
    close(pd[1]);
    while (1) {
        char id;
        char size;
        char out[256];
        read_fd(pd[0], &id, 1);
        read_fd(pd[0], &size, 1);
        read_fd(pd[0], out, size);
        for (i = 0; i < users; i++) {
            write_fd(&id, client_socket[i]);
            write_fd(&size, client_socket[i]);
            write_fd(out, client_socket[i]);
        }
        
    }
    return OK;
}
