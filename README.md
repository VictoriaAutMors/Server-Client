# Server-Client
Server-Client implementation in C language. An encrypted chat service for local network

# Server-Client supports:

1) RSA cipher encryption
2) shows time and nickname of the user
3) send messages from one client to another
4) up to 100 clients

# Installation:

1) download files 
2) run in terminal: 
``` 
make
```
It compiles C files from source folder and returns binary files in bin folder. It will create it if folder doesn't exist 

# How to launch server:

In bin folder:
run in terminal:
```
./server <port number> <number of clients>
```
* port number - Choose Port Number for the server in range 1500-8400. 
* number of clients - Choose Number of the clients from 2-100.

# How to launch client:

In bin folder:
run in terminal:
```
./client <ip> <port>
```
* ip - IP adress of the server.and 
* port -  port of the server. 
![alt text](https://github.com/VictoriaAutMors/Server-Client/tree/master/source/common/images/1.png "Server")
![alt text](https://github.com/VictoriaAutMors/Server-Client/tree/master/source/common/images/2.png "Server")
![alt text](https://github.com/VictoriaAutMors/Server-Client/tree/master/source/common/images/3.png "Server")

# RSA Encryption 
RSA (Rivest–Shamir–Adleman) is an algorithm used by modern computers to encrypt and decrypt messages. It is an asymmetric cryptographic algorithm. Asymmetric means that there are two different keys. This is also called public key cryptography, because one of the keys can be given to anyone. The other key must be kept private.

* At start client and server ask for two prime numbers less than 100 to generate private and public keys. 

# Clean: 

run in terminal: 
```
make clean
```
It deletes all files in bin folder and bin folder

# Additionaly python version of the chat in source folder
