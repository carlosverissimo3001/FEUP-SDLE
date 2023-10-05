CFLAGS = -lzmq

CLIENT = client
SERVER = server
PROXY = proxy

CC = clang
RM = rm -rf

all: $(CLIENT) $(SERVER) $(PROXY)

$(CLIENT):
	$(CC) $(CFLAGS) client.c -o client

$(SERVER):
	$(CC) $(CFLAGS) server.c -o server

$(PROXY):
	$(CC) $(CFLAGS) proxy.c -o proxy

fclean:
	$(RM) $(CLIENT) $(SERVER) $(PROXY)

re: fclean
	$(MAKE) all
