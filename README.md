# Server-Client
Server-Client implementation in C language. An encrypted chat service for local network

# Server-Client supports:
```
1) XOR cipher encryption
2) shows time and nickname of the user
3) send messages from one client to another
4) up to 100 clients
```
# Installation:
```
1) download files 
2) run in terminal: make
It compiles C files from source folder and returns binary files in bin folder. It will create it if folder doesn't exist 
```
# How to launch server:
```
In bin folder:
run in terminal:
./server <port number> <number of clients>

<port number> - Choose Port Number for the server in range 1500-8400. 
<number of clients> - Choose Number of the clients from 2-100.
```
# How to launch client:
```
In bin folder:
run in terminal:
./client <ip> <port> <key>
<ip> - IP adress of the server.
<port> -  port of the server. 
<key> - it's key to encryption. All clients should have the same one to work properly
```
# Clean: 
```
run in terminal: make clean
It deletes all files in bin folder and bin folder
```
