#ifndef COMMON_H
#define COMMON_H
/* GLOBAL VARIABLES */
#define NR_OF_CS 2
#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100

#include <painlessMesh.h>



/* STRUCT TO REPRESENT A POSITION IN THE SYSTEM */
struct Position
{
    int x;
    int y;
};

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

/* NODE ATTRIBUTES */
int distance;
int range;
int load;
float battery_level;
float battery_consumption;
Position current_position;
Position destination;

//statemachine functions
Position random_position();
Position nearest_charging_station();
void initialize_charging_stations();
void initialize_node();
bool move(Position dest, int cs);
int calc_distance();
float calc_battery_consumption();
int calc_range();
float calc_priority();
void print_info();
void handle_move_destination();
void handle_move_charging_station();
void handle_connect_and_broadcast(String zoneId);
void handle_queuing();
void handle_charging();
void update_values();
void stateMachine();


Task taskStateMachine(TASK_SECOND * 1, TASK_FOREVER, &stateMachine);

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