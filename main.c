//
// Created by cedricj on 12/22/22.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include "main.h"
#include "sensor_db.h"


pthread_t connmgr_thread;
pthread_t datamgr_thread;
pthread_t storagemgr_thread;

pthread_t writer_t;

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

    connmgr_args_t connmgrArg;
    connmgrArg.port = PORT;
    connmgrArg.buffer = buffer;



    // Create the threads
    pthread_create(&connmgr_thread, NULL, connmgr_main, (void*)&connmgrArg);

/*
    FILE* fp_sensor_data = fopen("sensor_data", "r");
    pthread_create(&writer_t, NULL, writer_thread, fp_sensor_data);
    pthread_join(writer_t, NULL);
    fclose(fp_sensor_data);
*/



   //pthread_create(&datamgr_thread,NULL,datamgr_parse_sensor_files(buffer),NULL);


   //pthread_create(&storagemgr_thread, NULL, sensor_db_main, (void*)buffer);
   pthread_join(connmgr_thread, NULL);
}

//create writer thread
void *writer_thread(void* fp_sensor_data) {
    //get sensor data from binary file
    sensor_data_t *sensor_data = malloc(sizeof(sensor_data_t));
    while (fread(&sensor_data->id, sizeof(sensor_data->id ), 1, fp_sensor_data) == 1) {
        fread(&sensor_data->value, sizeof(sensor_data->value), 1, fp_sensor_data);
        fread(&sensor_data->ts, sizeof(sensor_data->ts), 1, fp_sensor_data);

        //put sensor data into buffer
        sbuffer_insert(buffer, sensor_data);

    }

    //put dummy in file for reader thread to know when to stop
    sensor_data_t *dummy = malloc(sizeof(sensor_data_t));
    dummy -> id = 0;
    sbuffer_insert(buffer, dummy);
    free(dummy);

    free(sensor_data);


    pthread_exit(0);
}

