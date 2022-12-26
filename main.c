//
// Created by cedricj on 12/22/22.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"
#include <unistd.h>
#include <sys/types.h>

#define READ_END 0
#define WRITE_END 1


pthread_t connmgr_thread;
pthread_t datamgr_thread;
pthread_t storagemgr_thread;

pthread_mutex_t logger_mutex;

sbuffer_t *buffer;

int seq = 0;


int PORT;

int main(int argc, char *argv[]) {

    //Get port from arguments
    if (argc != 2) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 65536\n");
        exit(EXIT_FAILURE);
    }

    PORT = atoi(argv[1]);
    if (PORT > 65536 || PORT < 1024) {
        printf("Need to pass a portnumber larger than 1023, and smaller than 65536\n");
        exit(EXIT_FAILURE);
    }

    //create pipes
    int fd[2];
    int ret = pipe(fd);
    if (ret == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // In the child process
    if (pid == 0) {
        // Close write end of the pipe
        close(fd[WRITE_END]);


        pthread_mutex_init(&logger_mutex, NULL);


        while(1) {
            // Read from the pipe
            char read_msg[1024];
            long bytes_read = read(fd[READ_END], read_msg, sizeof(read_msg));

            // If the pipe is closed, exit the loop
            if (bytes_read == 0) {
                break;
            }

            //get current time
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char time[100];
            sprintf(time, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour + 1, tm.tm_min,
                    tm.tm_sec);

            //log <sequence number> <timestamp> <read_msg> to a new line to file
            char *s = malloc(strlen(read_msg) + strlen(time) + 1);

            //lock mutex
            pthread_mutex_lock(&logger_mutex);

            FILE *fp;
            fp = fopen("gateway.log", "a+");

            sprintf(s, "%d %s %s\n", seq, time, read_msg);
            fprintf(fp, "%s", s);
            seq++;

            //close file
            fclose(fp);

            //unlock mutex
            pthread_mutex_unlock(&logger_mutex);

            free(s);
        }





    }

        // In the parent process
    else {

        // Initialize the buffer
        sbuffer_init(&buffer);

        //initialize the semaphore
        sem_t *sem = malloc(sizeof(sem_t));
        sem_init(sem, 0, 1);

        connmgr_args_t connmgrArg;
        connmgrArg.port = PORT;
        connmgrArg.buffer = buffer;
        connmgrArg.pipe_write_fd = fd[WRITE_END];

        pthread_create(&connmgr_thread, NULL, connmgr_main, (void *) &connmgrArg);

        datamgr_args_t datamgrArgs;
        datamgrArgs.buffer = buffer;
        datamgrArgs.sem = sem;
        datamgrArgs.pipe_write_fd = fd[WRITE_END];

        pthread_create(&datamgr_thread, NULL, datamgr_parse_sensor_files, (void *) &datamgrArgs);


        storagemgr_args storagemgrArgs;
        storagemgrArgs.buffer = buffer;
        storagemgrArgs.sem = sem;
        storagemgrArgs.pipe_write_fd = fd[WRITE_END];


        pthread_create(&storagemgr_thread, NULL, sensor_db_main, (void *) &storagemgrArgs);


        pthread_join(connmgr_thread, NULL);
        pthread_join(datamgr_thread, NULL);
        pthread_join(storagemgr_thread, NULL);

        free(sem);
    }
}
