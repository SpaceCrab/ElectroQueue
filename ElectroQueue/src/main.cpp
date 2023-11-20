//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
//#include <scenario-simulation.cpp>


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

String prevZoneID ;

bool networkstate = false;

uint32_t testMessagesSent = 0;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

void sendMessage() ; // Prototype so PlatformIO doesn't complain
void updatePosition();

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
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
Task taskUpdateposition(TASK_SECOND * 1, TASK_FOREVER, &updatePosition);
Task taskStateMachine(TASK_SECOND * 1, TASK_FOREVER, &stateMachine);

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

    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    taskSendMessage.enableIfNot();
    networkstate = true;
  }
}

// test function to set a random position
void updatePosition()
{
  Serial.println("updated position ");
 // posY = 4;
  //posX = 4; 

  //chooses a random position 
  posY = random(4,6);
  posX = random(4,6);

  Serial.print(posX);
  Serial.println(posY);

  String str = "free memory: ";
  str += ESP.getFreeHeap();
  Serial.println(str);

  //check which zone the nodes is in
  //TODO: make this into a switch or some type of handler for multiple zones
  if(posY == ZONE_A_Y && posX == ZONE_A_X ) {
    Serial.println("in zone A");
    if(prevZoneID != ZONE_A_ID) { //check for zone change 
      exitZone();                 //leave the old network
      enterZone(ZONE_A_ID);       // create or enter the new network
      Serial.println("entered zone A from ");
      Serial.print(prevZoneID);
    }
    else{
      Serial.println("same zone, nothing changed ");
    }
    prevZoneID = ZONE_A_ID;
  }
  else if (posY == ZONE_B_Y && posX == ZONE_B_x ){
    Serial.println("in zone B");
    if(prevZoneID != ZONE_B_ID){
      exitZone();
      enterZone(ZONE_B_ID);
      Serial.println("Entered zone B from ");
      Serial.print(prevZoneID);
    }
    else{
      Serial.println("same zone, nothing changed ");
    }
    prevZoneID = ZONE_B_ID;
  }
  else{
    prevZoneID = "not a zone";
    Serial.println("leaving zone");
    exitZone();
  }
}

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

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);
//  mesh.onReceive(&receivedCallback);
//  mesh.onNewConnection(&newConnectionCallback);
//  mesh.onChangedConnections(&changedConnectionCallback);
//  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  /*userScheduler.addTask(taskStateMachine);
  taskStateMachine.enable();
  taskStateMachine.setInterval(10000);*/

  Serial.println("creating scheduler tasks ");
  Serial.println("taskSendmessage");
  userScheduler.addTask( taskSendMessage );

  Serial.println("taskUpdatePosition");
  userScheduler.addTask(taskUpdateposition);
  taskUpdateposition.setInterval(30000);

  Serial.println("taskUpdatePosition enable");
  taskUpdateposition.enable();

  //Serial.println("taskSendMessage enable");
  //taskSendMessage.enable();

  Serial.println("init complete");
}

void loop() {
  mesh.update();
}

