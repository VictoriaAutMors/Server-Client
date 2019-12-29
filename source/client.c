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
#include <math.h>

enum errors {
    OK,
    PUBLIC,
    PRIVATE,
    CLIENT_EXIT,
    ERR_READ,
    ERR_REALLC,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT,
    ERR_SEND,
    ERR_RECV,
    ERR_KILL_PROC,
    CLOSED_CONCT,
    DISCNT
};

double private_key, public_key, server_key, server_val;
int val;

int socket_init(void);
struct sockaddr_in server_addres_init(int port, char *ip);
void connect_server(int client_socket, struct sockaddr_in addr);
int send_message(char *message, int client_socket);
int recv_message(int client_socket);
void kill_child_proc(pid_t pid);
void readf(ssize_t fd, void *message, int sock_size);
void sendf(int client_socket, char *buffer, int len, int flag);
int generate_key(int, int);
char *encrypt(char *, double, double);
char *decrypt(char *, int);
int encryption_init();
int is_prime(int);
int gcd(int, int);
void arg_check(int argc);

int main(int argc, char **argv) {
    arg_check(argc);
	int client_socket, port = atoi(argv[2]);
    char buffer[1000], *ip = argv[1];
    pid_t pid;
	struct sockaddr_in serverAddr;
	client_socket = socket_init();
    serverAddr = server_addres_init(port, ip);
    connect_server(client_socket, serverAddr);
    if ((pid = fork()) == 0) {
        while(1) {
            if (send_message(buffer, client_socket) == DISCNT) {
                close(client_socket);
                puts("[-]Disconnected from server.");
                exit(CLIENT_EXIT);
            }
        }
    }
    while(1) { 
        if (recv_message(client_socket) != OK) {
            break;
        }
	}
    kill_child_proc(pid);
    close(client_socket);
	return 0;
}

/* initialize socket. Exit with ERR_SOCKET and error message on error */
int socket_init(void) {
    int tmp = socket(AF_INET, SOCK_STREAM, 0);
	if (tmp < 0) {
		err(ERR_SOCKET, "[-]Error to create socket.");
	}
    puts("[+]Client Socket is created.");
    return tmp;
}

/* initialize server address */
struct sockaddr_in server_addres_init(int port, char *ip) {
    struct sockaddr_in serverAddr;
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ip);
    return serverAddr;
}

/* connect to the server. EXit with ERR_CONNECT and error message on error*/
void connect_server(int client_socket, struct sockaddr_in addr) {
    int tmp;
    char nickname[100], skey[100], ser_val[100], ser_key[100], sval[100];
    fputs("Please enter your nickname in this chat: ", stdout);
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
	printf("[+]Connected to Server. Everyone see you as a %s\n", nickname);
    val = encryption_init();
    if (gcvt(public_key, 100, skey) == NULL) {
        err(ERR_READ, NULL);
    }
    sprintf(sval, "%d", val);
    readf(client_socket, ser_val, 100);
    readf(client_socket, ser_key, 100);
    sendf(client_socket, sval, 100, 0);
    sendf(client_socket, skey, 100, 0);
    sendf(client_socket, nickname, strlen(nickname), 0);
    server_key = atoi(ser_val);
    server_val = atoi(ser_key);

}

/* read message from fd. Exit ERR_READ and error message on error */
void readf(ssize_t fd, void *message, int sock_size) {
    if (read(fd, message, sock_size) < 0) {
        err(ERR_READ, NULL);
    }
}

/* Sends message to the socket. EXit with ERR_SEND and error message on error */
void sendf(int client_socket, char *buffer, int len, int flag) {
    if (send(client_socket, buffer, len, flag) < 0) {
        err(ERR_SEND, "error, failed to sent message");
    }
}

/* Sends message to the server. EXit with ERR_SEND and error message on error */
int send_message(char *message, int client_socket) {
    int len;
    time_t my_time = time(NULL);
    char *encrypted = NULL, *time_str = ctime(&my_time);
    time_str[strlen(time_str) - 1] = '\0';
    if (fgets(message, sizeof(message), stdin) == NULL) {
        err(ERR_RECV, NULL);
    }
    len = strlen(message);
    message[len - 1] = '\0';
    if (strcmp(message, "/exit") == 0) {
        return DISCNT;
    }
    printf("[%s] sended: %s\n", time_str, message);
    encrypted = encrypt(message, server_val, server_key);
    sendf(client_socket, message, len, 0);
    free(encrypted);
    return OK;
}

/* Receive message from the server. EXit with ERR_RECV and error message on error */
int recv_message(int client_socket) {
    ssize_t size_msg, size_nick;
    time_t my_time = time(NULL);
    char nickname[100], *decrypted = NULL,
                            message[1000], *time_str = ctime(&my_time);
    time_str[strlen(time_str) - 1] = '\0';
    size_nick = recv(client_socket, &nickname, 100, 0);
    size_msg = recv(client_socket, &message, 1000, 0);
    if (size_msg < 0 || size_nick < 0) {
        err(ERR_RECV, "[-]Error in receiving data.");
    } else if (size_msg == 0) {
        printf("Server closed connection\n");
        return CLOSED_CONCT;
    } else {
        decrypted = decrypt(message, val);
        printf("[%s] received a message from %s: %s\n", time_str, nickname, message);
    }
    free(decrypted);
    return OK;
}

char *decrypt(char *message, int val) {
    int i = 0;
    char *decrypted = NULL;
    do {
        decrypted = (char *)realloc(decrypted, (i +1) * sizeof(char));
        if (decrypted == NULL) {
            err(ERR_REALLC, NULL);
        }
        decrypted[i] = pow(message[i], private_key);
        decrypted[i] = fmod(decrypted[i], val);
        i++;
    } while(decrypted[i - 1] != '\0');
    return decrypted;
}

char *encrypt(char *message, double val, double key) {
    int i = 0;
    char *encrypted = NULL;
    do {
        encrypted = (char *)realloc(encrypted, (i + 1) * sizeof(char));
        if (encrypted == NULL) {
            err(ERR_REALLC, NULL);
        }
        if (message[i] != '\0') {
            encrypted[i] = pow(message[i], key);
            encrypted[i] = fmod(encrypted[i], val);
        } else {
            encrypted[i] = '\0';
        }
        i++;
    } while(encrypted[i - 1] != '\0');
    return encrypted;
}

/* initialize encyption. returns product of multiplication two prime numbers.
It needs to encypt and decrypt messages */
int encryption_init() {
    int num1, num2, val;
    double totient;
    puts("Enter 2 prime numbers less than 100 to initialize server encyption");
    if (scanf("%d %d", &num1, &num2) == EOF) {
        err(ERR_READ, NULL);
    }
    if (is_prime(num1) || is_prime(num2)) {
        puts("Wrong input, one of the number is not prime");
        exit(ERR_INCORRECT_ARGS);
    }
    totient = (num1 - 1) * (num2 - 1);
    val = num1 * num2;
    public_key = generate_key(PUBLIC, totient);
    private_key = generate_key(PRIVATE, totient);
    return val;
}

int generate_key(int type, int totient) {
    int count, val = 2; 
    double key = 2;
    if (type == PUBLIC) {
        /* Choose key such that key > 1 and coprime to totient 
        which means gcd (key, totient) must be equal to 1, 
        key is the public key */
        while(key < totient) {
            count = gcd(key,totient);
            if (count == 1) {
                break;
            } else {
                key++;
            }
        }
    } else {
        /* Choose key such that it satisfies the equation:
        key * public_key = 1 + k * (totient),
        key is the private key not known to everyone. */
        key = (1 + (val * totient)) / public_key;
    }
    return key;
}

/* functions checks number. if number is prime it returns 0. 
Returns 1 in other case*/
int is_prime(int num) {
    if (num <= 1) {
        return 0;
    } 
    if (num % 2 == 0 && num > 2) return 0;
    for (int i = 3; i < num / 2; i+= 2) {
        if (num % i == 0) {
            return 1;
        }
    }
    return 0;
}

/* functions find greatest common divisor of a and b */
int gcd(int a, int b) {
    while (a != b) {
        if (a > b) {
            return gcd(a - b, b);
        } else {
            return gcd(a, b - a);
        }
    }
    return a;
}

/* function will kill child process if it's still working */
void kill_child_proc(pid_t pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG) == 0) {
        if (kill(pid, SIGKILL) < 0) {
            err(ERR_KILL_PROC, NULL);
        }
    }
}

/* checks argument number. Exit with ERR_INCORRECT_ARGS on wrong amount*/
void arg_check(int argc) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5005");
        exit(ERR_INCORRECT_ARGS);
    }
}