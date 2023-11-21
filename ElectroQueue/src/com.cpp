#include <painlessMesh.h>
//#include <com.h>
//#include <state.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_X        5
#define   ZONE_B_ID       "zone B"
#define   ZONE_A_ID       "zone A"

bool networkstate = false;
String prevZoneId ;

painlessMesh  mesh;



struct response{
    u_int32_t nodeId;
    bool higher;
};

std::list<response> responseList;

std::list<u_int32_t> nodeList;

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

String getPrevZoneId (){
    return prevZoneId;
}

std::list<response> getResponseList(){
    return responseList;
}

std::list<u_int32_t> getNodeList(){
    return nodeList;
}

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

void meshSetup(){
    
}

bool meshInit(String prefix, String password, int port){
  if(!networkstate){
    mesh.init( prefix, password, &userScheduler, port );
    networkstate = true;
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    return true;
  } 
  return false;
}

bool exitZone(){
  if(networkstate){
    taskSendMessage.disable();
    mesh.stop();
    networkstate = false;
    Serial.println("left network");
    return true;
  }
  return false;
}

bool enterZone(String zoneID){
  if(!networkstate)
  {
    Serial.println("entered zone ");
    Serial.println(zoneID);

    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    taskSendMessage.enableIfNot();
    networkstate = true;
    return true;
  }
  return false;
}

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  Serial.println("sent message");
  // test code 
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}