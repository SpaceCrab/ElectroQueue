//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include "state.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_x        5
#define   ZONE_A_ID       "zone A"
#define   ZONE_B_ID       "zone B"

int posY = 0;
int posX = 0;
int queuePos = 0;

state currentState = connect_broadcast;

struct response
{
  u_int32_t nodeID;
  bool higher;
};



std::list<response> responseList;
std::list<u_int32_t> nodeList;
std::list<u_int32_t>::iterator nodeListIt;

String prevZoneID ;

bool networkstate = false;

uint32_t testMessagesSent = 0;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

void sendMessage() ; // Prototype so PlatformIO doesn't complain

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());


}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  nodeList = mesh.getNodeList();
  setNodeList(nodeList);
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void meshInit(String prefix, String password, int port){
  if(!networkstate){
    mesh.init( prefix, password, &userScheduler, port );
    networkstate = true;
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  } 

}
// User stub

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskStateCheck(TASK_SECOND * 1, TASK_FOREVER, &stateCheck);
Task taskSendBroadcast(TASK_SECOND * 1, TASK_FOREVER, &sendBroadcast);

void stateCheck(){

  String zoneId; 
  currentState = updateState();

  switch (currentState)
  {
  case connect_broadcast:
    zoneId = getPos();
    enterZone(zoneId);

    taskSendBroadcast.setIterations(4);
    taskSendBroadcast.enable();
    break;
  case charging:
    break;
  case queueing:
    break;
  default:
    break;
  }
}

void sendBroadcast(){
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  testMessagesSent++;
  Serial.println("sent message");
}

void addToList(u_int32_t nodeID, bool higherScore){
  // add a node and its response to the responseList
  struct response newResponse;

  newResponse.higher = higherScore;
  newResponse.nodeID = nodeID;

  std::list<response>::iterator responseListIt = responseList.begin();
  responseList.insert(responseListIt, newResponse);
}

void removeFromList(u_int32_t id){
  //remove a node and its response from the response list  
  std::list<response>::iterator responseListIt;
  for (responseListIt = responseList.begin(); responseListIt != responseList.end(); ++responseListIt){
    if(responseListIt->nodeID == id)responseList.erase(responseListIt) ;
  }
}

bool nodesInNetwork(){
  nodeList = mesh.getNodeList();
  if(nodeList.empty()) return true;
  else return false;
}

void exitZone(){
  if(networkstate){
    taskSendMessage.disable();
    mesh.stop();
    networkstate = false;
    Serial.println("left network");
  }
}

void enterZone(String zoneID){
  if(!networkstate)
  {
    Serial.println("entered zone ");
    Serial.println(zoneID);

    Serial.println("starting network.....");
    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    networkstate = true;

    nodeList = mesh.getNodeList();

    Serial.println("broadcasting request to charge");
    /*if(nodeList.empty()){ //checks if the network has any other nodes in it 
      Serial.println("broadcasting request to charge");
      taskSendMessage.enable();
    }*/
  }
}

// test function to set a random position
void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  testMessagesSent++;
  Serial.println("sent message");

  // test code 
  
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);

  Serial.println("creating scheduler tasks ");
  Serial.println("taskSendmessage");
  userScheduler.addTask( taskSendMessage );

  userScheduler.addTask(taskStateCheck);
  taskStateCheck.setInterval(100);
  taskStateCheck.enable();

  userScheduler.addTask(taskSendBroadcast);

  Serial.println("init complete");
}

void loop() {
  mesh.update();
}

