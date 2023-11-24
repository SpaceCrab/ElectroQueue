//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include <string>
#include <iostream>

#include "state.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555
#define ZONE_A_Y 4
#define ZONE_A_X 4
#define ZONE_B_Y 5
#define ZONE_B_x 5
#define ZONE_A_ID "zone A"
#define ZONE_B_ID "zone B"
#define BROADCAST_PREFIX "BROADCAST"
#define SINGLE_PREFIX "SINGLE"

int posY = 0;
int posX = 0;
int queuePos = 0;

String currentMsg;
u_int32_t currentTarget;
String zoneId;

state currentState = move_to_destination;

struct response
{
  u_int32_t nodeID;
  bool higher;
};

std::list<response> responseList;
std::list<u_int32_t> nodeList;
std::list<u_int32_t>::iterator nodeListIt;

String prevZoneID;

bool networkstate = false;

uint32_t testMessagesSent = 0;

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void sendMessage(); // Prototype so PlatformIO doesn't complain
void sendBroadcast();
void sendSingleMsg();
void stateCheck();

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
Task taskStateCheck(TASK_SECOND * 1, TASK_FOREVER, &stateCheck);
Task taskSendBroadcast(TASK_SECOND * 1, TASK_FOREVER, &sendBroadcast);
Task taskSendSingle(TASK_SECOND * 1, TASK_FOREVER, &sendSingleMsg);

void addToList(u_int32_t nodeID, bool higherScore)
{
  // add a node and its response to the responseList
  struct response newResponse;

  newResponse.higher = higherScore;
  newResponse.nodeID = nodeID;

  std::list<response>::iterator responseListIt = responseList.begin();
  responseList.insert(responseListIt, newResponse);
}

void removeFromList(u_int32_t id)
{
  // remove a node and its response from the response list
  std::list<response>::iterator responseListIt;
  for (responseListIt = responseList.begin(); responseListIt != responseList.end(); ++responseListIt)
  {
    if (responseListIt->nodeID == id)
      responseList.erase(responseListIt);
  }
}

void compareList()
{
  nodeList = mesh.getNodeList();
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

void updateOrAddNodeID(uint32_t targetNodeID, bool newHigherValue)
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
int qeueuValue(){
  int val = 0;
  for(auto &resp : responseList){
    if(!resp.higher) val++;
  }
  return val;
}

// returns true if no other node has a higher priority in the queue
bool allResponseTrue(){
  for(auto &resp: responseList){
    if(!resp.higher) return false;
  }
  return true;
}

void printNodeList()
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
void printResponseList()
{
  // Iterate through the list and print each element
  for (const auto &resp : responseList)
  {
    Serial.print("ResponseList");
    Serial.println("NodeID: ");
    Serial.print(resp.nodeID);
    Serial.print(", Higher: ");
    Serial.println(resp.higher);
  }

  // Add a separator for better readability
  Serial.println("------");
}

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  /*when the recieved msg is a broadcast with prio score, compare the score.
      if the score is lower than the own score: respond with false and add it to the responselist
      if the score is higher than the own score: respond with true and add it to the responselist
  */
  if (currentState != queuing)
    return;
  if (msg.startsWith(BROADCAST_PREFIX))
  {
    Serial.println("Broadcast message recieved");
    float ownScore = calc_prio();
    char delimiter = ',';

    int pos = msg.indexOf(delimiter);
    if (pos != -1)
    {
      String scoreStr = msg.substring(pos + 1);
      int otherScore = scoreStr.toInt();

      if (ownScore > otherScore)
      {
        currentMsg = "SINGLE,FALSE";
        currentTarget = from;
        taskSendSingle.setIterations(3);
        taskSendSingle.enable();
        updateOrAddNodeID(from, true);
      } // send response with false;

      if (ownScore < otherScore)
      {
        currentMsg = "SINGLE,TRUE";
        currentTarget = from;
        taskSendSingle.setIterations(3);
        taskSendSingle.enable();
        updateOrAddNodeID(from, false);
      } // send response with true;
    }
  }
  /*when the recieved msg is a response: store the response in the responselist
   */
  else if (msg.startsWith(SINGLE_PREFIX))
    ;
  {
    bool response = false;
    if (msg.endsWith("TRUE"))
      response = true;
    updateOrAddNodeID(from, response);
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  nodeList = mesh.getNodeList();
  setNodeList(nodeList);
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void meshInit(String prefix, String password, int port)
{
  if (!networkstate)
  {
    mesh.init(prefix, password, &userScheduler, port);
    networkstate = true;
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  }
}
// User stub

void sendBroadcast()
{
  mesh.sendBroadcast(currentMsg);
  Serial.println("sent broadcast message");
}

void sendSingleMsg()
{
  mesh.sendSingle(currentTarget, currentMsg);
}

bool nodesInNetwork()
{
  nodeList = mesh.getNodeList();
  if (nodeList.empty())
    return true;
  else
    return false;
}

void exitZone()
{
  if (networkstate)
  {
    taskSendMessage.disable();
    mesh.stop();
    networkstate = false;
    Serial.println("left network");
  }
}

void enterZone(String zoneID)
{
  if (!networkstate)
  {
    Serial.println("entered zone ");
    Serial.println(zoneID);

    Serial.println("starting network.....");
    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    networkstate = true;

    nodeList = mesh.getNodeList();

    // Serial.println("broadcasting request to charge");
    if (nodeList.empty())
    { // checks if the network has any other nodes in it
      Serial.println("broadcasting request to charge");
      taskSendMessage.enable();
    }
  }
}

// test function to set a random position
void sendMessage()
{
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  testMessagesSent++;
  Serial.println("sent message");

  // test code

  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void stateCheck()
{
  Serial.println("checking state");
  currentState = update_state();
  Serial.println(currentState);
  // printResponseList();
  // printNodeList();

  switch (currentState)
  {
  case connect_and_broadcast:
    // returns a struct -> position = {x,y}
    zoneId = String(get_curr_pos().x + "I" + get_curr_pos().y);
    Serial.println("zone id");
    Serial.println(zoneId);
    connecting();
    enterZone(zoneId);
    if (!nodeList.empty())
    {
      String prio = String(calc_prio());
      currentMsg = "BROADCAST," + prio; // replace with real score from state machine
      taskSendBroadcast.setIterations(4);
      taskSendBroadcast.enable();
      broadcastComplete();
    }
    else
      Serial.println("no other nodes");
    break;
  case queuing:
    compareList();
    ready_to_charge(allResponseTrue());
    break;
  case charging:
    break;
  default:
    exitZone();
    break;
  }
}
// Needed for painless library

void setup()
{
  Serial.begin(115200);
  initialize_charging_stations();
  Serial.println("statemachine init");
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);
  networkstate = true;

  initialize_node(mesh.getNodeId());
  Serial.println("creating scheduler tasks ");
  Serial.println("taskSendmessage");
  userScheduler.addTask(taskSendMessage);

  Serial.println("adding tasks");
  userScheduler.addTask(taskStateCheck);
  taskStateCheck.setInterval(100);
  Serial.println("statemachine enable");
  taskStateCheck.enable();
  Serial.println("init done");

  userScheduler.addTask(taskSendSingle);

  userScheduler.addTask(taskSendBroadcast);

  Serial.println("init complete");
  exitZone();
}

void loop()
{
  mesh.update();
}
