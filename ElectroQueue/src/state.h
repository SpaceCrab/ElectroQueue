#ifndef STATE_H
#define STATE_H


/* GLOBAL VARIABLES */
#define NR_OF_CS 2
#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100



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


#endif