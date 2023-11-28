#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <painlessMesh.h>

struct response
{
  u_int32_t nodeID;
  bool higher;
};

void addToList(u_int32_t nodeID, bool higherScore, std::list<response>& responseList);
void removeFromList(u_int32_t id, std::list<response>& responseList);
void comparelist(std::list<u_int32_t>& nodeList, std::list<response>& responseList);
void updateOrAddNodeId(u_int32_t targetNodeId, bool newHigherValue, std::list<response>& responseList);
int queueValue(std::list<response>& responseList);
bool allResponseTrue(std::list<response>& responseList);
void printResponseList(std::list<response>& responseList);
void printNodeList(std::list<u_int32_t>& nodeList);

#endif