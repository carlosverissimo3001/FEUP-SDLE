CFLAGS = -lzmq
SUBSCRIBER = subscriber
PUBLISHER = publisher

CC = clang
RM = rm -rf

all: $(SUBSCRIBER) $(PUBLISHER)

$(SUBSCRIBER):
	$(CC) $(CFLAGS) subscriber.c -o subscriber

$(PUBLISHER):
	$(CC) $(CFLAGS) publisher.c -o publisher

fclean:
	$(RM) 

re: fclean
	$(MAKE) all