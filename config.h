/**
 * \author Cédric Josse
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>
#include <string.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

/**
 * structure to hold sensor data
 */
typedef struct {
    sensor_id_t id;         /** < sensor id */
    sensor_value_t value;   /** < sensor value */
    sensor_ts_t ts;         /** < sensor timestamp */
    int read_by_datamgr;    /** < flag to indicate if the sensor data has been read by datamgr */
    int read_by_storagemgr; /** < flag to indicate if the sensor data has been read by storagemgr */
} sensor_data_t;

typedef struct {
    sensor_id_t id;         /** < sensor id */
    sensor_value_t value;   /** < sensor value */
    sensor_ts_t ts;         /** < sensor timestamp */
    int read_by_datamgr;    /** < flag to indicate if the sensor data has been read by datamgr */
    int read_by_storagemgr; /** < flag to indicate if the sensor data has been read by storagemgr */
} sensor_data2_t;

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

typedef struct{
    sensor_id_t sensor_id;         /** < sensor id */
    uint16_t room_id ;             /** < room id */
    sensor_value_t r_AVG[RUN_AVG_LENGTH];   /** < sensor value running average */
    int head;                      /** < index of the first element in the queue */
    int tail;                      /** < index of the next empty slot in the queue */
    sensor_ts_t last_ts;         /** < sensor timestamp */
} element_t;



#endif /* _CONFIG_H_ */
