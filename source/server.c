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
#include <math.h>

enum errors {
    OK,
    PUBLIC,
    PRIVATE,
    ERR_REALLC,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_PIPE,
    ERR_SEND,
    ERR_READ,
    ERR_CLOSE_PIPE,
    ERR_WRITE,
    ERR_LIST,
};

double private_key, public_key;

sem_t semaphore;

int socket_init(int socket_option);
int server_init(int port, int clients);
void reciever(int id, int input, int output);
void writef(ssize_t fd, void *message, int sock_size);
void readf(ssize_t fd, void *message, int sock_size);
void close_pipe(int pd[2]);
int generate_key(int, int);
char *encrypt(char *, char *, char *);
char *decrypt(char *, int);
int encryption_init();
int is_prime(int);
int gcd(int, int);
void arg_check(int argc);

int main(int argc, char **argv) {
    arg_check(argc);
    char client_val[100][LOGIN_MAX], client_key[100][LOGIN_MAX], vals[LOGIN_MAX], skey[LOGIN_MAX];
    int server_socket, client_socket[LOGIN_MAX], pd[2], port, val,
                    id = 0, sock_port = atoi(argv[1]), clients = atoi(argv[2]);
    ssize_t size1;
    socklen_t sock_size;
    char nicknames[100][NAME_MAX], message[LINE_MAX], *dec_msg = NULL, *enc_msg = NULL, *addr = NULL;
    struct sockaddr_in client[100];
    struct sockaddr *client_ptr[100];
    server_socket = server_init(sock_port, clients);
    val = encryption_init();
    sprintf(vals, "%d", val);
    if (gcvt(public_key, LOGIN_MAX, skey) == NULL) {
        err(ERR_INCORRECT_ARGS, NULL);
    }
    for (int i = 0; i < clients; i++) {
        client_ptr[i] = (struct sockaddr *)&client[i];
    }
    puts("wait for connect");
    for (int i = 0, j = 0; i < clients; i++, j++) {
        client_socket[i] = accept(server_socket, client_ptr[i], &sock_size);
        addr = inet_ntoa(client[i].sin_addr);
        port = ntohs(client[i].sin_port);
        printf("connected: %s %d \n ", addr, port);
        writef(client_socket[i], vals, LOGIN_MAX);
        writef(client_socket[i], skey, LOGIN_MAX);
        readf(client_socket[i], client_val[i], LOGIN_MAX);
        readf(client_socket[i], client_key[i], LOGIN_MAX);
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
    puts("starting chat");
    while (1) {
        readf(pd[0], &id, sizeof(id));
        readf(pd[0], &size1, sizeof(size1));
        readf(pd[0], message, size1);
        dec_msg = decrypt(message, val);
        printf("got from pipe client %s id %d, content %s\n", nicknames[id],
                                                                    id, message);
        for (int i = 0; i < clients; i ++) {
            if(i != id) {
                enc_msg = encrypt(dec_msg, client_val[id], client_key[id]);
                printf("sending '%s' to client %d\n", message, i);
                writef(client_socket[i], nicknames[id], sizeof(nicknames[id]));
                writef(client_socket[i], message, strlen(message));
            }
        }
        free(dec_msg);
        free(enc_msg);
    }
    puts("stopping chat");
    close_pipe(pd);
    return OK;
}

/* initialize socket. Exit with ERR_SOCKET and error message on error */
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

char *encrypt(char *message, char *cval, char *ckey) {
    int val = atoi(cval), key = atoi(ckey), i = 0;
    char *encrypted = NULL;
    do {
        encrypted = (char *)realloc(encrypted, (i +1) * sizeof(char));
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

/* initialize server */
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

/* receives messages then writes them in pipe */
void reciever(int id, int input, int output) {
    size_t sock_size;
    char message[100];
    while((sock_size = read(input ,&message, 100)) > 0) {
        printf("got %zu bytes from client socket %d, content %s\n", sock_size, id, message);
        sem_post(&semaphore);
        writef(output ,&id ,sizeof(id));
        writef(output ,&sock_size ,sizeof(sock_size));
        writef(output ,message ,sock_size);
        sem_wait(&semaphore);
    }
}

/* read message from fd. Exit ERR_READ and error message on error */
void readf(ssize_t fd, void *message, int sock_size) {
    if (read(fd, message, sock_size) < 0) {
        err(ERR_READ, NULL);
    }
}

/* write message in fd. Exit with  ERR_WRITE and error message on error */
void writef(ssize_t fd, void *message, int sock_size) {
    if (write(fd, message, sock_size) < 0) {
        err(ERR_WRITE, "Failed to write in socket");
    }
}

/* close pipes. Exit with ERR_CLOSE_PIPE and error message on error  */
void close_pipe(int pd[2]) {
    if (close(pd[1]) < 0) {
        err(ERR_CLOSE_PIPE, NULL);
    }
    if (close(pd[0]) < 0) {
        err(ERR_CLOSE_PIPE, NULL);
    }
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

/* checks argument number. Exit with ERR_INCORRECT_ARGS on wrong amount*/
void arg_check(int argc) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <clients number>");
        puts("Example:");
        puts("./server 8000 5");
        exit(ERR_INCORRECT_ARGS);
    }
}
