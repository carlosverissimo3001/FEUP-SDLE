CFLAGS = -lzmq
SUBSCRIBER = subscriber

PUBLISHER_PT = publisher_pt
PUBLISHER_US = publisher_us

CC = clang
RM = rm -rf

all: $(SUBSCRIBER) $(PUBLISHER_US) $(PUBLISHER_PT)

$(SUBSCRIBER):
	$(CC) $(CFLAGS) subscriber.c -o subscriber

$(PUBLISHER_US):
	$(CC) $(CFLAGS) publisher_us.c -o publisher_us

$(PUBLISHER_PT):
	$(CC) $(CFLAGS) publisher_pt.c -o publisher_pt

fclean:
	$(RM) subscriber publisher_pt publisher_us

re: fclean
	$(MAKE) all
