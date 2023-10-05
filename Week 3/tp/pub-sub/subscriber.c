//  Weather update client
//  Connects SUB socket to tcp://localhost:5556
//  Collects weather updates and finds avg temp in zipcode
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "zhelpers.h"

struct place {
    char *zipcode;
    char *temp_scale;
};

int main (int argc, char *argv [])
{
    const struct place nyc = {"10001", "F"};
    const struct place opo = {"4200", "C"};   
    const struct place places[2] = {nyc, opo};
    
    int rc = -1;

    //  Socket to talk to server
    void *context = zmq_ctx_new ();
    printf ("Collecting updates from weather server...\n");
    
    // connect to pt server
    void *sub_pt = zmq_socket (context, ZMQ_SUB);
    rc = zmq_connect (sub_pt, "tcp://localhost:5557");
    assert (rc == 0);
    rc = zmq_setsockopt (sub_pt, ZMQ_SUBSCRIBE, opo.zipcode, strlen (opo.zipcode));
    assert (rc == 0);


    // connect to us server
    void *sub_us = zmq_socket (context, ZMQ_SUB);
    rc = zmq_connect (sub_us, "tcp://localhost:5557");
    assert (rc == 0);
    rc = zmq_setsockopt (sub_us, ZMQ_SUBSCRIBE, nyc.zipcode, strlen (nyc.zipcode));
    assert (rc == 0);

    //  Process 100 updates
    int update_nbr;
    long total_temp[2] = {0};

    zmq_pollitem_t items[] = {
        { sub_us, 0, ZMQ_POLLIN, 0 },
        { sub_pt, 0, ZMQ_POLLIN, 0 }
    };

    for (update_nbr = 0; update_nbr < 100; update_nbr++) {
        zmq_poll(items, 2, -1);
        // printf("Update no. %d: %s\n", update_nbr, string);

        // receive us update
        if (items[0].revents & ZMQ_POLLIN){
            char *string = s_recv(sub_us);
            if (string != NULL){
                int zipcode, temperature, relhumidity;
                sscanf (string, "%d %d %d", &zipcode, &temperature, &relhumidity);
                total_temp[0] += temperature;
                free(string);
            }
        }

        // receive pt update
        if (items[1].revents & ZMQ_POLLIN){
            char *string = s_recv(sub_pt);
            if (string != NULL){
                int zipcode, temperature, relhumidity;
                sscanf (string, "%d %d %d", &zipcode, &temperature, &relhumidity);
                total_temp[1] += temperature;
                free(string);
            }
        }

    }
    
    //  Calculate and report average temp
    printf ("Average temperature for zipcode '%s' was %d%s\n", nyc.zipcode, (int) (total_temp[0] / update_nbr), nyc.temp_scale);
    printf ("Average temperature for zipcode '%s' was %d%s\n", opo.zipcode, (int) (total_temp[1] / update_nbr), opo.temp_scale);

    zmq_close (sub_us);
    zmq_close (sub_pt);

    zmq_ctx_destroy (context);
    return 0;
}