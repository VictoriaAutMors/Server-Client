#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <err.h>
#include <sys/wait.h>
#include <limits.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_PIPE,
    ERR_SEND,
    ERR_READ,
    ERR_CLOSE_PIPE,
    ERR_WRITE,
    ERR_LIST,
};

sem_t semaphore;

/* initialize socket. Exit with ERR_SOCKET and error message on error */
int socket_init(int socket_option);

/* initialize server */
int server_init(int port, int clients);

/* receives messages then writes them in pipe */
void reciever(int id, int input, int output);

/* additional functions */

/* write message in fd. Exit with  ERR_WRITE and error message on error */
void writef(ssize_t fd, void *message, int sock_size);

/* read message from fd. Exit ERR_READ and error message on error */
void readf(ssize_t fd, void *message, int sock_size);

/* close pipes. Exit with ERR_CLOSE_PIPE and error message on error  */
void close_pipe(int pd[2]);

/* checks argument number. Exit with ERR_INCORRECT_ARGS on wrong amount*/
void arg_check(int argc);

int main(int argc, char **argv) {
    arg_check(argc);
    int server_socket, client_socket[100], pd[2], port,
                    id = 0, sock_port = atoi(argv[1]), clients = atoi(argv[2]);
    ssize_t size1;
    socklen_t sock_size;
    char nicknames[100][NAME_MAX], buf[LINE_MAX], *addr = NULL;
    struct sockaddr_in client[100];
    struct sockaddr *client_ptr[100];
    server_socket = server_init(sock_port, clients);
    for (int i = 0; i < clients; i++) {
        client_ptr[i] = (struct sockaddr *)&client[i];
    }
    for (int i = 0, j = 0; i < clients; i++, j++) {
        client_socket[i] = accept(server_socket, client_ptr[i], &sock_size);
        addr = inet_ntoa(client[i].sin_addr);
        port = ntohs(client[i].sin_port);
        printf("connected: %s %d \n ", addr, port);
        readf(client_socket[i], nicknames[j], NAME_MAX);
    }
    if (pipe(pd) < 0) {
        err(ERR_PIPE, NULL);
    }
    sem_init(&semaphore, 1, 0);
    for (int i = 0; i < clients; i++) {
        if(fork() == 0) {
            printf("client %d process started\n", i);
            reciever(i, client_socket[i], pd[1]);
            close(client_socket[i]);
            close_pipe(pd);
            return OK;
        }
    }
    wait(NULL);
    puts("starting chat");
    while (1) {
        readf(pd[0], &id, sizeof(id));
        readf(pd[0], &size1, sizeof(size1));
        readf(pd[0], buf, size1);
        printf("got from pipe client %s id %d, content %s\n", nicknames[id],
                                                                    id, buf);
        for (int i = 0; i < clients; i ++) {
            printf("sending '%s' to client %d\n", buf, i);
            writef(client_socket[i], nicknames[id], sizeof(nicknames[id]));
            writef(client_socket[i], buf, size1);
        }
    }
    puts("stopping chat");
    close_pipe(pd);
    return OK;
}

int socket_init(int socket_option) {
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        err(ERR_SOCKET, "Failed to initialize socket");
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                                                    sizeof(socket_option)) < 0) {
        err(ERR_SOCKET, "Failed to set socket option");
    }
    return server_socket;
}

int server_init(int port, int clients) {
    int server_socket = socket_init(1);
    struct sockaddr_in server_addr;
    // set socket address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // start listen mode
    if (listen(server_socket, clients) < 0) {
        err(ERR_LIST, "Failed to prepare connection");
    }
    return server_socket;
}

void reciever(int id, int input, int output) {
    size_t sock_size;
    char buf[LINE_MAX];
    while((sock_size = read(input ,&buf, LINE_MAX)) > 0) {
        printf("got %zu bytes from client socket %d, content %s\n", sock_size, id, buf);
        sem_post(&semaphore);
        writef(output ,&id ,sizeof(id));
        writef(output ,&sock_size ,sizeof(sock_size));
        writef(output ,buf ,sock_size);
        sem_wait(&semaphore);
    }
}

void readf(ssize_t fd, void *message, int sock_size) {
    if (read(fd, message, sock_size) < 0) {
        err(ERR_READ, NULL);
    }
}

void writef(ssize_t fd, void *message, int sock_size) {
    if (write(fd, message, sock_size) < 0) {
        err(ERR_WRITE, "Failed to write in socket");
    }
}

void close_pipe(int pd[2]) {
    if (close(pd[1]) < 0) {
        err(ERR_CLOSE_PIPE, NULL);
    }
    if (close(pd[0]) < 0) {
        err(ERR_CLOSE_PIPE, NULL);
    }
}

void arg_check(int argc) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <clients number>");
        puts("Example:");
        puts("./server 8000 5");
        exit(ERR_INCORRECT_ARGS);
    }
}
