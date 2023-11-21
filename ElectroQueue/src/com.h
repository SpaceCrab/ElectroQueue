#ifndef COM_H
#define COM_H



#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_X        5
#define   ZONE_B_ID       "zone B"
#define   ZONE_A_ID       "zone A"


String prevZoneID ;

painlessMesh  mesh;

Scheduler userScheduler; // to control your personal task

struct response{
    u_int32_t nodeId;
    bool higher;
};

std::list<response> responseList;

std::list<u_int32_t> nodeList;

//void meshInit(const String &prefix, const String &password, int port);
void meshInit(String prefix, String password, int port);
void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void exitZone();
void enterZone(String zoneID);

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );


#endif 
