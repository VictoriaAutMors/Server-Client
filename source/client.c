#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <limits.h>
#include <err.h>
#include <signal.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT,
    ERR_SEND,
    ERR_RECV,
    ERR_KILL_PROC,
    CLOSED_CONCT,
    DISCNT
};

int socket_init(void);

struct sockaddr_in server_addres_init(int port, char *ip);

void connect_server(int client_socket, struct sockaddr_in addr);

int send_message(char *message, int client_socket, int key);

int recv_message(int client_socket, int key);

/* function will kill child process if it's still working */
void kill_child_proc(pid_t pid);

void sendf(int client_socket, char *buffer, int len, int flag);

void arg_check(int argc);

int main(int argc, char **argv) {
    arg_check(argc);
	int clientSocket, port = atoi(argv[2]), key = atoi(argv[3]);
    char buffer[LINE_MAX], *ip = argv[1];
    pid_t pid;
	struct sockaddr_in serverAddr;
	clientSocket = socket_init();
    serverAddr = server_addres_init(port, ip);
    connect_server(clientSocket, serverAddr);
    if ((pid = fork()) == 0) {
        while (1) {
            if (send_message(buffer, clientSocket, key) == DISCNT) {
                return OK;
            }
        }
    }
    while (1) {
        if (recv_message(clientSocket, key) != OK) {
            break;
        }
	}
    kill_child_proc(pid);
    close(clientSocket);
	return 0;
}

int socket_init(void) {
    int tmp = socket(AF_INET, SOCK_STREAM, 0);
	if (tmp < 0) {
		err(ERR_SOCKET, "[-]Error to create socket.");
	}
    puts("[+]Client Socket is created.");
    return tmp;
}

struct sockaddr_in server_addres_init(int port, char *ip) {
    struct sockaddr_in serverAddr;
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ip);
    return serverAddr;
}

void connect_server(int client_socket, struct sockaddr_in addr) {
    int tmp;
    char nickname[NAME_MAX];
    fputs("Please enter your nickname in this chat:", stdout);
    if (scanf("%s", nickname) == EOF) {
        if (ferror(stdin) == -1) {
            clearerr(stdin);
            err(ERR_RECV, "Failed to get nickname");
        }
    }
	tmp = connect(client_socket, (struct sockaddr*)&addr, sizeof(addr));
	if (tmp < 0) {
		err(ERR_CONNECT, "[-]Error in connection.");
	}
	puts("[+]Connected to Server.");
    sendf(client_socket, nickname, strlen(nickname), 0);
}

void sendf(int client_socket, char *buffer, int len, int flag) {
    if (send(client_socket, buffer, len, flag) < 0) {
        err(ERR_SEND, "error, failed to sent message");
    }
}

int send_message(char *message, int client_socket, int key) {
    int len;
    time_t my_time = time(NULL);
    char encrypted[LINE_MAX], *time_str = ctime(&my_time);
    time_str[strlen(time_str) - 1] = '\0';
    printf("[%s]", time_str);
    fputs(">>", stdout);
    if (fgets(message, sizeof(message), stdin) == NULL) {
        err(ERR_RECV, NULL);
    }
    len = strlen(message);
    message[len - 1] = '\0';
    for (int i = 0; i < len; i++) {
        encrypted[i] = (char)(((int)message[i])^key);
    }
    sendf(client_socket, encrypted, len, 0);        
    if (strcmp(message, ":exit") == 0) {
        close(client_socket);
        puts("[-]Disconnected from server.");
        return DISCNT;
    }
    return OK;
}

int recv_message(int client_socket, int key) {
    int len;
    ssize_t size_msg, size_nick;
    time_t my_time = time(NULL);
    char nickname[NAME_MAX], decrypted[LINE_MAX], 
                            message[LINE_MAX], *time_str = ctime(&my_time);
    time_str[strlen(time_str) - 1] = '\0';
    size_nick = recv(client_socket, &nickname, NAME_MAX, 0);
    size_msg = recv(client_socket, &message, LINE_MAX, 0);
    len = strlen(message);
    for (int i = 0, j = 0; i < len; i++, j++) {
        if (message[i] == ' ') {
            decrypted[j] = message[i];
        } else {
            decrypted[j] = (char)(((int)message[i])^key);
        }
    }
    if (size_msg < 0 || size_nick < 0) {
        err(ERR_RECV, "[-]Error in receiving data.");
    } else if (size_msg == 0) {
        printf("Server closed connection\n");
        return CLOSED_CONCT;
    } else {
        message[size_msg] = '\0';
        puts(nickname);
        printf("[%s] %s\n", time_str, decrypted);
    }
    return OK;
}

void kill_child_proc(pid_t pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG) == 0) {
        if (kill(pid, SIGKILL) < 0) {
            err(ERR_KILL_PROC, NULL);
        }
    }
}

void arg_check(int argc) {
    if (argc != 4) {
        puts("Incorrect args.");
        puts("./client <ip> <port> <key>");
        puts("Example:");
        puts("./client 127.0.0.1 5005 8194");
        exit(ERR_INCORRECT_ARGS);
    }
}