//
// Created by cedricj on 12/19/22.
//

#ifndef PLAB3CONNMGR_TEST_SERVER_H
#define PLAB3CONNMGR_TEST_SERVER_H

#include "sbuffer.h"
#include "lib/tcpsock.h"

typedef struct arg_struct {
    tcpsock_t *sock;
    sbuffer_t *buffer;
}arg_t;

typedef struct connmgr_args{
    int port;
    sbuffer_t * buffer;
}connmgr_args_t;


void* socket_thread(arg_t *arg);

void * connmgr_main(void *arg);

#endif //PLAB3CONNMGR_TEST_SERVER_H
