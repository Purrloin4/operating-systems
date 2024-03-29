/**
 * \author Cédric Josse
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>
#include <semaphore.h>

sem_t semaphore;
pthread_mutex_t mutex;
/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    //initialize semaphore and mutex
    sem_init(&semaphore, 0, 3);
    pthread_mutex_init(&mutex, NULL);

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    //destroy semaphore
    sem_destroy(&semaphore);
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    sem_wait(&semaphore);
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_FAILURE;

    //lock mutex
    pthread_mutex_lock(&mutex);
    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head == buffer->tail) // buffer has only one node
        {
        buffer->head = buffer->tail = NULL;
        }
    else  // buffer has many nodes empty
        {
        buffer->head = buffer->head->next;
        }
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    free(dummy);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    //lock mutex
    pthread_mutex_lock(&mutex);
    dummy->data = *data;
    dummy->next = NULL;
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    sem_post(&semaphore);
    return SBUFFER_SUCCESS;
}

int sbuffer_eof(sbuffer_t *buffer){
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_FAILURE;
    if (buffer->head->data.id == 0) return SBUFFER_SUCCESS;
    return SBUFFER_FAILURE;
}

int sbuffer_is_empty(sbuffer_t *buffer) {
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_SUCCESS;
    return SBUFFER_FAILURE;
}

int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data) {


    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_FAILURE;

    //lock mutex
    pthread_mutex_lock(&mutex);
    *data = buffer->head->data;
    //unlock mutex
    pthread_mutex_unlock(&mutex);

    return SBUFFER_SUCCESS;
}

int sbuffer_set_read_by_datamgr(sbuffer_t *buffer){

    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_FAILURE;

    //lock mutex
    pthread_mutex_lock(&mutex);
    buffer->head->data.read_by_datamgr = 1;
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    return SBUFFER_SUCCESS;
}

int sbuffer_set_read_by_storagemgr(sbuffer_t *buffer){

    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_FAILURE;

    //lock mutex
    pthread_mutex_lock(&mutex);
    buffer->head->data.read_by_storagemgr = 1;
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    return SBUFFER_SUCCESS;
}





