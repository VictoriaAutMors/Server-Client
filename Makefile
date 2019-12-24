all: 
	mkdir bin -p
	gcc ./source/server.c -o ./bin/server -Wall -Wextra -O2 -lpthread
	gcc ./source/client.c -o ./bin/client -Wall -Wextra -O2

clean:
	rm ./bin/client
	rm ./bin/server
	rmdir bin
