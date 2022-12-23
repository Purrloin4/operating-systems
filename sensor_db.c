//
// Created by cedricj on 11/16/22.
//




#ifndef PLAB1DATAMGR_SENSOR_DB_H
#define PLAB1DATAMGR_SENSOR_DB_H


#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>






/*
open a text file with a given name, and providing an indication
if the file should be overwritten if the file already exists or if the data should
be appended to the existing file
 */
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
    int x = fprintf(f, "%d,%f,%ld", id, value, ts);
    if (x < 0){
        printf("Error writing to file");
        return -1;
    }
    if(x > 0){
        printf("Data written to file");
        return x;
    }
}


//close the cvs file

int close_db(FILE * fp){
    fclose(fp);
    printf("The file has been closed");
}



#endif //PLAB1DATAMGR_SENSOR_DB_H

