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

