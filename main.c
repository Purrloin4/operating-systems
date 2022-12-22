//
// Created by cedricj on 12/22/22.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include <unistd.h>


pthread_t connmgr_thread;
pthread_t datamgr_thread;
pthread_t storagemgr_thread;

sbuffer_t *buffer;

int main() {

    // Initialize the buffer
    sbuffer_init(&buffer);

    // Create the threads
    pthread_create(&connmgr_thread, NULL, , NULL);
    pthread_create(&datamgr_thread, NULL, datamgr_parse_sensor_data, NULL);
    pthread_create(&storagemgr_thread, NULL, storagemgr_parse_sensor_data, NULL);



}
