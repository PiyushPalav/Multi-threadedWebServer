
CC = g++ -Wno-write-strings
SERVER_FILE = simple-server.cpp
CLIENT_FILE = simple-client.cpp
HTTP_SERVER_FILE = http_server.cpp

all: server client

server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -lpthread -o server

client: $(CLIENT_FILE)
	$(CC) $(CLIENT_FILE) -o client

clean:
	rm -f server client