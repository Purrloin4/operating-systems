/**
 * \author Cédric Josse
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "datamgr.h"
#include "lib/dplist.h"
#include "sbuffer.h"
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>


dplist_t * sensor_list;
int pipefd;

void * element_copy(void * element) {
    element_t* copy = malloc(sizeof (element_t));
    assert(copy != NULL);
    memcpy(copy, element, sizeof(element_t));
    return (void *) copy;
}

void element_free(void ** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    if (((element_t*)x)->sensor_id < ((element_t*)y)->sensor_id) {
        return -1;
    } else if (((element_t*)x)->sensor_id > ((element_t*)y)->sensor_id) {
        return 1;
    } else {
        return 0;
    }
}

void* datamgr_parse_sensor_files(void *args){

    printf("---------datamgr_parse_sensor_files start------------\n");

    struct datamgr_args * arg = (struct datamgr_args *) args;
    sem_t* sem = arg->sem;
    sbuffer_t * buffer = arg->buffer;
    pipefd = arg->pipe_write_fd;

    //open file "room_sensor.map"
    FILE *fp_sensor_map = fopen("room_sensor.map", "r");
    //if fp_sensor_map is NULL, print error message and exit
    if (fp_sensor_map == NULL) {
        fprintf(stderr,"error: fp_sensor_map is NULL");
    }

    //put room_id and sensor_id in the list
    sensor_list = dpl_create(&element_copy, &element_free, &element_compare);
    element_t *element = malloc(sizeof(element_t));
    int room_id;
    int sensor_id_from_map;
    while (fscanf(fp_sensor_map, "%d %d", &room_id, &sensor_id_from_map) == 2) {
        element->sensor_id = sensor_id_from_map;
        element->room_id = room_id;
        sensor_list = dpl_insert_at_index(sensor_list, element, -1, true);
    }
    free(element);


        //get sensor data from buffer and update the list
        sensor_data_t *sensor_data = malloc(sizeof(sensor_data_t));
    while (sbuffer_eof(buffer) != SBUFFER_SUCCESS  ) {
        while (sbuffer_is_empty(buffer) == SBUFFER_FAILURE) {
            sem_wait(sem);
            if (sbuffer_eof(buffer) == SBUFFER_SUCCESS) {
                sem_post(sem);
                break;
            }

            sbuffer_read(buffer, sensor_data);
            if(sensor_data->read_by_datamgr==1) {
                sem_post(sem);
            }else{
                if (sensor_data->read_by_storagemgr==1) {
                    sbuffer_remove(buffer, sensor_data);
                }


                sbuffer_set_read_by_datamgr(buffer);

                //find the element in the list
                element_t *element1 = get_element_fromID(sensor_data->id);

                if (element1 == 0){
                    sem_post(sem);
                    break;
                }

                //update the average and the timestamp
                putr_AVG(element1, sensor_data->value);

                element1->last_ts = datamgr_get_last_modified(element1, sensor_data->ts);


                //check average for logging
                double avg = datamgr_get_avg(element1->sensor_id);
                if (avg > SET_MAX_TEMP) {
                    fprintf(stderr, "Room %d is too hot\n", element1->room_id);
                    char message[100];
                    sprintf(message, "Sensor node %d reports it’s too hot (avg temp = %f)", element1->sensor_id, avg);
                    write(pipefd, message, sizeof(message));
                }

                if (avg < SET_MIN_TEMP) {
                    fprintf(stderr, "Room %d is too cold\n", element1->room_id);
                    char message[100];
                    sprintf(message, "Sensor node %d reports it’s too cold (avg temp = %f)", element1->sensor_id, avg);
                    write(pipefd, message, sizeof(message));
                }
                sem_post(sem);
            }
        }
    }



    free(sensor_data);
    datamgr_free();
    pthread_exit(NULL);

};

element_t *get_element_fromID(sensor_id_t sensor_id) {

    int index = dpl_get_index_of_element(sensor_list, &sensor_id);
    if (index == -1) {
        char message[100];
        sprintf(message, "Received sensor data with invalid sensor node ID %hu", sensor_id);
        write(pipefd, message, sizeof(message));
        return 0;
    } else {
        element_t *element = dpl_get_element_at_index(sensor_list, index);
        return element;
    }

}




sensor_value_t datamgr_get_avg(sensor_id_t sensor_id){

        element_t *element = get_element_fromID(sensor_id);


        //get the average value of the arrar r_AVG
        double sum = 0;
        for(int x=0;x<RUN_AVG_LENGTH;x++){
            sum = sum + element->r_AVG[x];
        }

        int count = 0;
        for(int i=0;i<RUN_AVG_LENGTH;i++){
            if(element->r_AVG[i]!=0){
                count++;
            }
        }

        double avg = sum/count;
        return avg;

}

void putr_AVG(element_t *element, sensor_value_t value){
    element->r_AVG[element->tail++]=value;
    element->tail%=RUN_AVG_LENGTH;
}

void datamgr_free(){

    dpl_free(&sensor_list, true);

}

time_t datamgr_get_last_modified(element_t* element,sensor_ts_t ts){

    if (element->last_ts > ts) {
        return element->last_ts;
    } else {
        return ts;
    }
}

