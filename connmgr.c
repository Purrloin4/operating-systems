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
int PORT;
int thread_id;


/**
 * Implements a sequential test server (only one connection at the same time)
 */

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 6553\n");
        exit(EXIT_FAILURE);
    }

    PORT = atoi(argv[1]);

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

        //start socket thread
        pthread_create(&threads[thread_id], NULL, socket_thread, client);

    } while (1);
    tcp_close(&client);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    return 0;
}

void* socket_thread(void* client) {
    int bytes,result;
    sensor_data_t data;
    int thread_id2 = thread_id;
    thread_id++;
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
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                    (long int) data.ts);
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




