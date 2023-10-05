CFLAGS = -lzmq

CLIENT = client
SERVER = server

CC = clang
RM = rm -rf

all: $(CLIENT) $(SERVER) 

$(CLIENT):
	$(CC) $(CFLAGS) client.c -o client

$(SERVER):
	$(CC) $(CFLAGS) server.c -o server

fclean:
	$(RM) $(CLIENT) $(SERVER)

re: fclean
	$(MAKE) all
