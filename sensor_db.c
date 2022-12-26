//
// Created by cedricj on 11/16/22.
//




#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>
#include "sensor_db.h"
#include "sbuffer.h"
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>

int fd;


void* sensor_db_main(void* args) {

    printf("--------------------sensor_db_main---------------------\n");


    struct storagemgr_args * arg = (struct storagemgr_args *) args;
    sem_t* sem = arg->sem;
    sbuffer_t * buffer = arg->buffer;
    fd = arg->pipe_write_fd;

    FILE *fp = open_db("data.csv", false);

    char message[100];
    sprintf(message, "A new data.csv file has been created.");
    write(fd, message, sizeof(message));

    sensor_data_t *sensor_data = malloc(sizeof(sensor_data_t));
        while (sbuffer_eof(buffer) != SBUFFER_SUCCESS  ) {
            while (sbuffer_is_empty(buffer) == SBUFFER_FAILURE) {
                //printf("at semwait in sensor_db_main\n");
                sem_wait(sem);
                if (sbuffer_eof(buffer) == SBUFFER_SUCCESS) {
                    break;
                }
                sbuffer_read(buffer, sensor_data);
                if (sensor_data->read_by_storagemgr == 1) {
                    sem_post(sem);
                }else{
                if (sensor_data->read_by_datamgr == 1) {
                    sbuffer_remove(buffer, sensor_data);
                }
                sbuffer_set_read_by_storagemgr(buffer);
                insert_sensor(fp, sensor_data->id, sensor_data->value, sensor_data->ts);


                sprintf(message, "Data insertion from sensor %d succeeded", sensor_data->id);
                write(fd, message, sizeof(message));

                sem_post(sem);
                }
            }
        }


    close_db(fp);
    char* message2 = "The data.csv file has been closed.";
    write(fd, message2, sizeof(message2));

    free(sensor_data);
    pthread_exit(NULL);
}




FILE * open_db(char * filename, bool append){
    FILE *fp;
    if(append){
        fp = fopen(filename, "a+");
        printf("Existing file opened for appending");
    }else{
        fp = fopen(filename, "w");
        printf("New file created");
    }
    if (fp == NULL){
        printf("Error opening file");
        printf("Error opening file");
        exit(1);
    }
    return fp;
}



//append a single sensor reading to the csv file
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){


    int x = fprintf(f, "%d,%f,%ld\n", id, value, ts);
    if (x < 0){
        printf("Error writing to file");
        return -1;
    }
    if(x > 0){
        printf("Data written to file: %d,%f,%ld\n", id, value, ts);
        return x;
    }
    return 0;
}


//close the cvs file

int close_db(FILE * fp){
    fclose(fp);
    printf("The file has been closed");
    return 1;
}


