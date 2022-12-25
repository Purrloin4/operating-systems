//
// Created by cedricj on 11/16/22.
//

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>
#include "sbuffer.h"
#include "semaphore.h"

typedef struct storagemgr_args{
    sem_t* sem;
    sbuffer_t * buffer;
}storagemgr_args;

void* sensor_db_main(void* arg);
FILE * open_db(char * filename, bool append);
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);
int close_db(FILE * f);