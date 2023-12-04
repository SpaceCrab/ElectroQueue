#include <painlessMesh.h>
#include "utils.hpp"

void addToList(u_int32_t nodeID, bool higherScore,std::list<response>& responseList)
{
  // add a node and its response to the responseList
  struct response newResponse;

  newResponse.higher = higherScore;
  newResponse.nodeID = nodeID;

  std::list<response>::iterator responseListIt = responseList.begin();
  responseList.insert(responseListIt, newResponse);
}

void removeFromList(u_int32_t id, std::list<response>& responseList)
{
    // remove a node and its response from the response list
    auto responseListIt = responseList.begin();
    while (responseListIt != responseList.end())
    {
        if (responseListIt->nodeID == id)
        {
            responseListIt = responseList.erase(responseListIt);
        }
        else
        {
            ++responseListIt;
        }
    }
}

void compareList(std::list<u_int32_t>& nodeList, std::list<response>& responseList)
{
  for (auto it = responseList.begin(); it != responseList.end();)
  {
    if (!std::count(nodeList.begin(), nodeList.end(), it->nodeID))
    {
      it = responseList.erase(it); // removes nodes not connected to the network
    }
    else
    {
      it++;
    }
  } // test
}

void updateOrAddNodeId(uint32_t targetNodeID, bool newHigherValue, std::list<response>& responseList)
{
  for (auto &resp : responseList)
  {
    if (resp.nodeID == targetNodeID)
    {
      resp.higher = newHigherValue; // Update the boolean value
      return;                       // Exit the function once the update is done
    }
  }

  // If the nodeID is not found, add a new element to the list
  responseList.push_back({targetNodeID, newHigherValue});
}

//returns nr of nodes with a higher priority
int queueValue(std::list<response>& responseList){
  int val = 0;
  for(auto &resp : responseList){
    if(!resp.higher) val++;
  }
  return val;
}

// returns true if no other node has a higher priority in the queue
bool allResponseTrue(std::list<response>& responseList){
  
  for(auto &resp: responseList){
    if(!resp.higher) return false;
  }
  return true;
}

void printNodeList(std::list<u_int32_t>& nodeList)
{
  Serial.println("nodelist:");
  // Iterate through the list and print each element
  for (const auto &nod : nodeList)
  {
    Serial.print("NodeID: ");
    Serial.println(nod);
  }

  // Add a separator for better readability
  Serial.println("------");
}
void printResponseList(std::list<response>& responseList)
{
  Serial.print("ResponseList");
  // Iterate through the list and print each element
  for (const auto &resp : responseList)
  {
    Serial.println("NodeID: ");
    Serial.print(resp.nodeID);
    Serial.print(", Higher: ");
    Serial.println(resp.higher);
  }

  // Add a separator for better readability
  Serial.println("------");
}
