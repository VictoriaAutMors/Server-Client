#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

void reciever ( int id , int input , int output) {
    size_t size;
    char buf[100];
    while ( (size = read ( input , &buf, 100)) > 0){
        printf("got %zu bytes from client socket %d, content %s\n", size, id, buf);
        /* critical start */
        write ( output , & id , sizeof ( id ));
        write ( output , & size , sizeof ( size ));
        write ( output , buf , size );
        /* critical stop */
    }
}

int main(int argc, char ** argv){
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));
    // set socket address
    struct sockaddr_in server_address ;
    server_address . sin_family = AF_INET ;
    server_address . sin_port = htons (8080);
    server_address . sin_addr . s_addr = INADDR_ANY ;
    bind ( server_socket , ( struct sockaddr *) & server_address , sizeof ( server_address ));
    // start listen mode
    listen ( server_socket , 5);

    int clientsNum = 2;
    struct sockaddr_in client[100] ;
    struct  sockaddr  * client_ptr[100];
    for(int i = 0; i < 100; i++){
        client_ptr[i] = (struct sockaddr *)&client[i];
    }
    socklen_t size ;
    int client_socket[100];
    char  nicks[100][100];
    int j = 0;
    for(int i = 0; i < clientsNum; i++){
        client_socket[i] = accept(server_socket, client_ptr[i], &size);
        char * addr = inet_ntoa ( client[i].sin_addr );
        int port = ntohs ( client[i].sin_port );
        printf ( "connected: %s %d \n " , addr , port );
        read(client_socket[i], nicks[j], 100);
        j++;
    }
    int receiver_2_sender[2];
    pipe ( receiver_2_sender );
    for ( int i = 0; i < clientsNum ; i++){
        if ( fork() == 0) {
            printf("client %d process started\n", i);
            reciever (i,  client_socket[i], receiver_2_sender[1]);
            close ( client_socket [i]);
            return 0;
        }
    }
    
    char buf[100];
    size_t size1;
    int id = 0;

    printf("starting main fan out\n");
    while (1) {
        read ( receiver_2_sender [0] , &id , sizeof ( id ));
        read ( receiver_2_sender [0] , &size1 , sizeof ( size1 ));
        read ( receiver_2_sender [0] , buf , size1 );
        printf("got from pipe client %s id %d, content %s\n", nicks[id], id, buf);
        for ( int i = 0; i < clientsNum ; i ++) {
            printf("sending '%s' to client %d\n", buf, i);
            write(client_socket[i], nicks[id], sizeof(nicks[id]));

            write(client_socket[i], buf, size1);

        }
    }
    printf("stopping main fan out\n");
    close ( receiver_2_sender [1]);
    close ( receiver_2_sender [0]);
    return 0;
}