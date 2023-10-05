#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "zhelpers.h"

int main (void) 
{
    void *context = zmq_ctx_new ();

    // This is our public endpoint for publishers (servers need to connect here)
    // Basically, the proxy is subscribing to the publishers, and then forwards it
    // to the subscribers
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    int f_rc = zmq_bind(frontend, "tcp://*:8100");
    assert (f_rc == 0);

    // This is our public endpoint for subscribers (clients need to connect here)
    void *backend = zmq_socket (context, ZMQ_XPUB);
    int b_rc = zmq_bind (backend, "tcp://*:8101");
    assert (b_rc == 0);

    printf("Proxy running...\n");

    // Run the proxy until the user interrupts us
    zmq_proxy (frontend, backend, NULL);

    
    zmq_close (frontend);
    zmq_close (backend);

    zmq_ctx_destroy (context);
    return 0;
}