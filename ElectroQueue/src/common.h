// common.h

#ifndef COMMON_H
#define COMMON_H

#include "painlessMesh.h"


/* Common functions used in both code1.cpp and code2.cpp */


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_X        5
#define   ZONE_A_ID       "zone A"
#define   ZONE_B_ID       "zone B"



/* GLOBAL VARIABLES */
#define NR_OF_CS 2
#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100



/* DEFINE STATES */
enum State
{
    assign_new_destination,
    move_to_destination,
    move_to_nearest_charging_station,
    connect_and_broadcast,
    queuing,
    charging
};


/* CONSTANT VALUE THAT REPRESENTS THE MAXIMUM BATTERY-CONSUMPTION FOR A NODE*/
const float max_battery_consumption = MAX_BATTERY_LEVEL / MAX_DISTANCE;

/* A LIST OF ALL CHARGING STATIONS "CS" IN THE SYSTEM */
Position charging_stations[NR_OF_CS];

/* */
int connected_nodes[5];

/* NODE ATTRIBUTES */
int distance;
int range;
int load;
float battery_level;
float battery_consumption;
int wait_count; // this will be changed to a variable that collects the time interval between each iteration (only when in state negotiate)
int broadcast_request;
float priority;
Position current_position;
Position destination;


/* SET THE INITIAL STATE */
State current_state = move_to_destination;
State next_state = move_to_destination;

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


/* STRUCT TO REPRESENT A POSITION IN THE SYSTEM */
struct Position
{
    int x;
    int y;
};

// Function declarations from code1.cpp
Position random_position();
Position nearest_charging_station();
void initialize_charging_stations();
void initialize_node();
void set_node(int lo, float bat_lev, float bat_con, int x_pos, int y_pos, int x_des, int y_des);
bool move(Position dest, int cs);
int calc_distance();
float calc_battery_consumption();
int calc_range();
float calc_priority();
void print_info();
void handle_move_destination();
void handle_move_charging_station();
void handle_connect_and_broadcast();
void handle_queuing();
void handle_charging();
void update_values();
void stateMachine();


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

#endif // COMMON_H
