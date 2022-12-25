/**
 * \author Cédric Josse
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <pthread.h>
#include "connmgr.h"
#include <semaphore.h>
#include <sys/select.h>
#include <unistd.h>

#ifndef TIMEOUT
#define TIMEOUT 5
#endif


#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting
pthread_t threads[MAX_CONN];  // array of threads
pthread_t timer_th[MAX_CONN];
pthread_t timer_th_server;
sem_t sem;  // semaphore to control the number of threads
int thread_id;
int flag[MAX_CONN];
int flag_server;
time_t last_data_time[MAX_CONN];
time_t last_data_time_server;



void * connmgr_main(void* conn_args) {

    printf("------ CONNMGR STARTED --------\n");

    struct connmgr_args* arg = (struct connmgr_args*) conn_args;
    int PORT = arg->port;
    sbuffer_t * buffer = arg->buffer;


    if(PORT>65536 || PORT<1024) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 6553\n");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port: %d\n",PORT);

    tcpsock_t *server, *client;
    thread_id = 0;
    sem_init(&sem, 0, MAX_CONN);

    pthread_create(&timer_th_server, NULL, timer_thread_server, NULL);


    printf("Server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_CONNECTION_PENDING) {
            printf("Incoming client connection\n");
            //printf("Starting thread with id: %d\n", thread_id);

            sem_wait(&sem);

            struct arg_struct args;
            args.sock = client;
            args.buffer = buffer;

            //start socket threa
            pthread_create(&threads[thread_id], NULL, (void *(*)(void *)) socket_thread, &args);
            pthread_create(&timer_th[thread_id], NULL, timer_thread, (void *) &thread_id);
        }

    } while (flag_server != 1);
    tcp_close(&client);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    //put dummy in buffer for datamgr and storagemgr threads to know when to stop
    sensor_data_t *dummy = malloc(sizeof(sensor_data_t));
    dummy -> id = 0;
    sbuffer_insert(buffer, dummy);
    free(dummy);
    return 0;
}



void *socket_thread(arg_t *arg) {
    int bytes,result;
    sensor_data_t data;
    int thread_id2 = thread_id;
    thread_id++;
    tcpsock_t *client = arg->sock;
    sbuffer_t *buffer = arg->buffer;
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);

        data.read_by_datamgr = 0;
        data.read_by_storagemgr = 0;
        if ((result == TCP_NO_ERROR) && bytes) {

            //instead of printing the data, we put it in the buffer
            printf("Connmgr has inserted :sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int) data.ts);
            sbuffer_insert(buffer, &data);

        }

        last_data_time[thread_id2] = time(NULL);
        last_data_time_server = time(NULL);


    } while (result == TCP_NO_ERROR && flag[thread_id2] != 1);

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else if (flag[thread_id2] == 1){
        printf("Timeout has closed peer connection\n");
    }
    else
        printf("Error occured on connection to peer\n");

    thread_id = thread_id2;
    //printf("Thread with id: %d is closing\n", thread_id);
    sem_post(&sem);

    pthread_exit(0);
}

void* timer_thread(void* arg) {
    // Extract the client socket descriptor from the argument
    int id = *((int*)arg);

    // Keep track of the last time data was received
    last_data_time[id] = time(NULL);

    while (1) {
        // Check if the timeout has expired
        time_t current_time = time(NULL);
        if (current_time - last_data_time[id] > TIMEOUT) {
            flag[id] = 1;
            break;
        }
        // Sleep for 1 second before checking again
        sleep(1);
    }
    pthread_exit(NULL);
}


void* timer_thread_server(void* arg) {


    // Set the timeout value
    const int timeout_secs = 20;
    // Keep track of the last time data was received
    last_data_time_server = time(NULL);

    while (1) {
        // Check if the timeout has expired
        time_t current_time = time(NULL);
        if (current_time - last_data_time_server > timeout_secs) {
            flag_server = 1;
            break;
        }
        // Sleep for 1 second before checking again
        sleep(1);
    }
    pthread_exit(NULL);
}

