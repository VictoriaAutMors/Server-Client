#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int key = 8194;

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    char nick[100];
    printf("Please enter your nickname in this chat:");
    scanf("%s", nick);
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");
    send(clientSocket, nick, strlen(nick), 0);
    if(fork() == 0){
        while (1) {
        	char buffer[1024];
            //scanf("%s", buffer);
            time_t my_time = time(NULL);
            char * time_str = ctime(&my_time);
            time_str[strlen(time_str) - 1] = '\0';
            printf("[%s]", time_str);
            printf(">>");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strlen(buffer) - 1] = '\0';
            char crypt[1000];
            for(int i = 0; i < strlen(buffer); i++){
                crypt[i] = (char)(((int)buffer[i])^key);
            }
            send(clientSocket, crypt, strlen(crypt), 0);        
               if(strcmp(buffer, ":exit") == 0){
                close(clientSocket);
                printf("[-]Disconnected from server.\n");
                exit(1);
            }
        }
        return 0;
    }
    char sender_nick[100];	
    while(1){
    	char buffer[1024];
        ssize_t size, size1;
        size1 = recv(clientSocket, sender_nick, 100, 0);
        size = recv(clientSocket, buffer, 1000, 0);
        char decrypt[1000];
        int l = 0;
        for(int i = 0; i < strlen(buffer); i++){
                if(buffer[i] == ' '){
                    decrypt[l] = buffer[i];
                    l++;
                }
                else{
                    decrypt[l] = (char)(((int)buffer[i])^key);
                    l++; 
                }
        }
		if(size < 0 || size1 < 0){
			printf("[-]Error in receiving data.\n");
            break;
		}else if (size == 0) {
            printf("Server closed connection\n");
            break;
        }else{
            buffer[size] = '\0';
			printf("%s: %s\n", sender_nick, decrypt);
		}
	}

	return 0;
}