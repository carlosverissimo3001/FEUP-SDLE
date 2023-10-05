//  Hello World client
// 1 on 1 chat client

#include <zmq.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "zhelpers.h"

#define HERE printf("Here\n");

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <wrt_port> <read_port>\n");
        return 1;
    }

    // create context
    void *context = zmq_ctx_new();

    // two sockets, one for sending, one for receiving, use REP/REQ
    void *sender = zmq_socket(context, ZMQ_REQ);
    void *receiver = zmq_socket(context, ZMQ_REP);

    // argv[1] -> writer port, argv[2] -> reader port
    char write_port[20];
    sprintf(write_port, "tcp://localhost:%s", argv[1]);

    char read_port[20];
    sprintf(read_port, "tcp://localhost:%s", argv[2]);
 
    // connect the sockets
    int rc = zmq_connect(sender, write_port);
    assert(rc == 0);

    rc = zmq_connect(receiver, read_port);
    assert(rc == 0);

    printf("Read Address: %s\n", read_port);
    printf("Write Address: %s\n", write_port);

    int stdin_fd = fileno(stdin);
    

    // poll stdin and the reader socket
    zmq_pollitem_t items[] = {
        {NULL, stdin_fd, ZMQ_POLLIN, 0},
        {receiver, 0, ZMQ_POLLIN, 0}
    };


    while (1) {
        zmq_poll(items, 2, -1);

        // flush stdin

        // poll the stdin
        if (items[0].revents & ZMQ_POLLIN) {
            printf("Something on stdin\n");
            
            // Handle stdin input
            char input[256];
            if (fgets(input, sizeof(input), stdin) != NULL) {
                printf("You entered: %s\n", input);
            } 
            
            // send the input to the other client
            s_send(sender, input);
        }


        // poll the reader socket
        if (items[1].revents & ZMQ_POLLIN) {
            printf("Something on the reader socket\n");
            char *input;
            
            // if there's smth on the reader socket, print it
            input = s_recv(receiver);
            printf("Received: %s\n", input);
            free(input);
        }
    }

}
