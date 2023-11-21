#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <painlessMesh.h>

enum state{
    charging,
    queueing,
    connect_broadcast
};

state updateState();
String getPos();

/*getter & setters*/
void setNodeList(std::list<u_int32_t> nodeList);


#endif 