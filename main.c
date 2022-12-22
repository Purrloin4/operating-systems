//
// Created by cedricj on 12/22/22.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include <unistd.h>


pthread_t connmgr_thread;
pthread_t datamgr_thread;
pthread_t storagemgr_thread;

sbuffer_t *buffer;

int PORT;

int main(int argc, char *argv[]) {

    //Get port from arguments
    if (argc != 2) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 65536\n");
        exit(EXIT_FAILURE);
    }

    PORT = atoi(argv[1]);

    if(PORT>65536 || PORT<1024) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 65536\n");
        exit(EXIT_FAILURE);
    }


    // Initialize the buffer
    sbuffer_init(&buffer);

    // Create the threads
    pthread_create(&connmgr_thread, NULL, connmgr_main(PORT, buffer), NULL);
   // pthread_create(&datamgr_thread, NULL, datamgr_parse_sensor_data, NULL);
   //pthread_create(&storagemgr_thread, NULL, storagemgr_parse_sensor_data, NULL);

}
