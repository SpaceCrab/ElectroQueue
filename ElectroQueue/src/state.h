#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <painlessMesh.h>

#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100
#define NR_OF_CS 2

struct position
{
    int x;
    int y;
};

enum state
{
    assign_new_destination,
    move_to_destination,
    move_to_charging_station,
    connect_and_broadcast,
    queuing,
    charging
};

state update_state();
position random_position();
position nearest_charging_station();
float calc_battery_consumption();
int calc_distance();
int calc_range();
float calc_prio();
boolean move(position dest, boolean cs);
void initialize_node();
void initialize_charging_stations();
void update_values();
void print_info();

state handle_move_destination();
state handle_move_charging_station();
state handle_charging();

/*setters*/
void setNodeList(std::list<u_int32_t> nodeList);
void set_state(state state);

/*getters*/

position get_curr_pos();
int get_id();
int get_load();
int get_place_in_queue();
float get_bat_lev();

#endif