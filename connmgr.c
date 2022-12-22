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
#include <unistd.h>


#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting
pthread_t threads[MAX_CONN];  // array of threads
sem_t sem;  // semaphore to control the number of threads
int thread_id;



void * connmgr_main(void* conn_args) {

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


    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        //printf("Starting thread with id: %d\n", thread_id);

        sem_wait(&sem);

        struct arg_struct args;
        args.sock = client;
        args.buffer = buffer;

        //start socket thread
        pthread_create(&threads[thread_id], NULL, (void *(*)(void *)) socket_thread, &args);

    } while (1);
    tcp_close(&client);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    return 0;
}

void* socket_thread(arg_t *arg) {
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
    } while (result == TCP_NO_ERROR);

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");

    thread_id = thread_id2;
    //printf("Thread with id: %d is closing\n", thread_id);
    sem_post(&sem);

    pthread_exit(0);
}




