CFLAGS = -lzmq (-Wall) -o
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
	$(RM) 

re: fclean
	$(MAKE) all

	