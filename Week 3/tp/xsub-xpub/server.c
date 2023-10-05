//  Weather update server
//  Binds PUB socket to tcp://*:5556
//  Publishes random weather updates
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "zhelpers.h"

int main (void) 
{
    void *context = zmq_ctx_new ();

    //  Socket to talk to clients through the proxy (proxy endpoint for publishers -> 8100)
    void *responder = zmq_socket (context, ZMQ_PUB);
    zmq_connect (responder, "tcp://localhost:8100");

    printf("Connected to proxy\n");


    while (1) {
        //  Wait for next request from client
        char *string = s_recv (responder);
        printf ("Received request: [%s]\n", string);
        free (string);

        //  Do some 'work'
        sleep (1);

        //  Send reply back to client
        s_send (responder, "World");
    }
    //  We never get here, but clean up anyhow
    zmq_close (responder);
    zmq_ctx_destroy (context);
    return 0;
}