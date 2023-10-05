//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "zhelpers.h"

struct Message {
    char *username;
    char *message;
    char *topic;
};

int main (void) 
{
    void *context = zmq_ctx_new ();

    //  Socket to talk to server (proxy endpoint for subscribers -> 8101)
    void *requester = zmq_socket (context, ZMQ_SUB);
    
    int c_rc = zmq_connect(requester, "tcp://localhost:8101");
    assert (c_rc == 0);

    printf("Connected to proxy\n");

    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        s_send (requester, "Hello");
        char *string = s_recv (requester);
        printf ("Received reply %d [%s]\n", request_nbr, string);
        free (string);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}

