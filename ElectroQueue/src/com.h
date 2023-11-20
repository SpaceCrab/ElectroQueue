
#ifndef COM_H
#define COM_H

#include <painlessMesh.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_X        5
#define   ZONE_A_ID       "zone A"
#define   ZONE_B_ID       "zone B"

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
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void meshInit(String prefix, String password, int port){
}
// User stub

// Function declarations from code2.cpp
//void meshInit(const String &prefix, const String &password, int port);
void sendMessage();
void updatePosition();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void exitZone();
void enterZone(String zoneID);

#endif 
