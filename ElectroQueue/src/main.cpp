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
#include <com.h>
#include <state.h>

String prevZoneID ;

uint32_t testMessagesSent = 0;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


//create tasks
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskStateMachine(TASK_SECOND * 1, TASK_FOREVER, &stateMachine);


// Needed for painless library

void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);
  
  userScheduler.addTask(taskStateMachine);
  taskStateMachine.enable();
  taskStateMachine.setInterval(10000);

  Serial.println("creating scheduler tasks ");
  Serial.println("taskSendmessage");
  userScheduler.addTask( taskSendMessage );

  //Serial.println("taskSendMessage enable");
  //taskSendMessage.enable();

  Serial.println("init complete");
}

void loop() {
  mesh.update();
}

