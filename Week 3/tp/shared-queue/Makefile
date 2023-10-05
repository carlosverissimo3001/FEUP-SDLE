CFLAGS = -lzmq

CLIENT = client
WORKER = worker
BROKER = broker

CC = clang
RM = rm -rf

all: $(CLIENT) $(WORKER) $(BROKER)

$(CLIENT):
	$(CC) $(CFLAGS) client.c -o client

$(WORKER):
	$(CC) $(CFLAGS) worker.c -o worker

$(BROKER):
	$(CC) $(CFLAGS) broker.c -o broker

fclean:
	$(RM) $(CLIENT) $(WORKER) $(BROKER)

re: fclean
	$(MAKE) all
