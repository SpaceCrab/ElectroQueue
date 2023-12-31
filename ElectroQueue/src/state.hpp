#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <painlessMesh.h>

#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100
#define NR_OF_CS 1

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
void broadcastComplete();
void chargingComplete();
void set_place_in_queue(int pos);
void connecting();
void ready_to_charge(bool queue_empty);
void set_id(int nodeId);
/*getters*/

position get_curr_pos();
int get_id();
int get_load();
int get_place_in_queue();
float get_bat_lev();
float get_prio_score();

// Print for OLED
void update_OLED();
void print_to_OLED_queue(int value);
void print_to_OLED_ID(int value);
void print_to_OLED_range(int value);
void print_to_OLED_dist(int value);
void print_to_OLED_consumption(float value);
void print_to_OLED_battery(float value);

void print_not_in_queue_OLED();

#endif