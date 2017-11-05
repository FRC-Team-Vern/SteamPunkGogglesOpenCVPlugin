#ifndef ZHELPERS_H
#define ZHELPERS_H

#include <zmq.h>


static int
s_send (void* socket, char* string) {
    int size = zmq_send (socket, string, strlen(string), 0);
    return size;
}

#endif // ZHELPERS_H

