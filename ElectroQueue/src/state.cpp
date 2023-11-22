#include "state.h"

state currentS = connect_broadcast;

std::list<u_int32_t> nodeListState;

void setNodeList(std::list<u_int32_t> newList){
    nodeListState = newList;
}

void setState(state state){
    currentState = state;
}

state updateState(){
    return currentState;
}


String getPos(){
    return "4.4";
}

int getPrio(){
    return 99;
}


