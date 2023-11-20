#include <com.h>
#include <painlessMesh.h>
#include <state.h>

bool networkstate = false;


void sendMessage() ; // Prototype so PlatformIO doesn't complain
void updatePosition();

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    nodeList = mesh.getNodeList();
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  nodeList = mesh.getNodeList();
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

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  Serial.println("sent message");

  // test code 
  
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}