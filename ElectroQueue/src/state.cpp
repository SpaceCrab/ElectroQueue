#include "state.h"

std::list<u_int32_t> nodeListState;

void setNodeList(std::list<u_int32_t> newList){
    nodeListState = newList;
}

state updateState(){
    return queueing;
}

String getPos(){
    return "4.4";
}

int getPrio(){
    return 99;
}


