#include "state.h"
#include <iostream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

const float max_battery_consumption = MAX_BATTERY_LEVEL / MAX_DISTANCE;

/* NODE ATTRIBUTES */
int distance;
int range;
int load;
float battery_level;
float battery_consumption;
int wait_count;
float priority;
int id;
int place_in_queue;
bool broadcast_complete = false;
bool first_in_queue = false;
position current_position;
position destination;
position charging_stations[NR_OF_CS];

state current_state = move_to_destination;
state next_state = move_to_destination;

std::list<u_int32_t> nodeListState;

void set_state(state state)
{
    current_state = state;
}

state update_state()
{
    switch (current_state)
    {
    case assign_new_destination:
        Serial.println("In state: assign new destination");
        destination = random_position();
        next_state = move_to_destination;
        break;

    case move_to_destination:
        Serial.println("In state: move to destination");
        next_state = handle_move_destination();
        break;

    case move_to_charging_station:
        Serial.println("In state: move to nearest charging destination");
        next_state = handle_move_charging_station();
        break;

    case connect_and_broadcast:
        Serial.println("In state: connect and broadcast");
        if(broadcast_complete)next_state = queuing;
        break;

    case queuing:
        Serial.println("In state: queuing");
        if(first_in_queue)next_state = charging;
        break;

    case charging:
        Serial.println("In state: charging");
        next_state = move_to_destination;
        break;
    }
    current_state = next_state;

    update_values();
    print_info();

    return current_state;
}

state handle_move_destination()
{

    if (range < distance)
    {
        return move_to_charging_station;
    }
    else
    {
        if (move(destination, false))
        {

            return assign_new_destination;
        }
        else
        {
            return move_to_destination;
        }
    }
}

state handle_move_charging_station()
{
    if (move(nearest_charging_station(), true))
    {
        return connect_and_broadcast;
    }
    else
    {
        return move_to_charging_station;
    }
}

void initialize_node(int id)
{
    load = std::rand() % MAX_LOAD + 1;
    battery_level = std::rand() % (MAX_BATTERY_LEVEL + 1);
    battery_consumption = calc_battery_consumption();
    current_position = random_position();
    destination = random_position();
    distance = calc_distance();
    range = calc_range();
}

void initialize_charging_stations()
{
    position cs = {8, 3};
    charging_stations[0] = cs;
    cs = {3, 8};
    charging_stations[1] = cs;
}

float calc_prio()
{
    float priority = 0;

    // Battery priority calculation
    float factor_battery_level = (MAX_BATTERY_LEVEL - battery_level) / MAX_BATTERY_LEVEL;

    if (battery_level >= 85)
    {
        priority += factor_battery_level * 3;
    }
    else if (battery_level <= 15)
    {
        priority += factor_battery_level * 3;
    }
    else
    {
        priority += factor_battery_level * 2;
    }

    // Range priority calculation
    float factor_range = (battery_consumption / max_battery_consumption);

    if (range > distance)
    {
        priority += factor_range * 1;
    }
    else if (range > (distance / 2))
    {
        priority += factor_range * 2;
    }
    else
    {
        priority += ((distance - range) / distance + factor_range) * 2;
    }

    return priority;
}

position random_position()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    position p = {std::rand() % 10, std::rand() % 10};
    return p;
}

float calc_battery_consumption()
{
    return ((((float)load / MAX_LOAD) * (max_battery_consumption - MIN_BATTERY_CONSUMPTION)) + MIN_BATTERY_CONSUMPTION);
}

int calc_range()
{
    return (std::floor(battery_level / battery_consumption));
}

int calc_distance()
{
    return (abs(current_position.x - destination.x) + abs(current_position.y - destination.y));
}

void update_values()
{
    distance = calc_distance();
    range = calc_range();
}

boolean move(position dest, boolean cs)
{
    if (dest.x != current_position.x)
    {
        current_position.x += (dest.x > current_position.x) ? 1 : -1;
        if (cs == 0)
        {
            battery_level -= battery_consumption;
        }
    }

    if (dest.y != current_position.y)
    {
        current_position.y += (dest.y > current_position.y) ? 1 : -1;
        if (cs == 0)
        {
            battery_level -= battery_consumption;
        }
    }

    return (dest.x == current_position.x) && (dest.y == current_position.y);
}

position nearest_charging_station()
{
    position cs = charging_stations[0];
    int min_distance = abs(current_position.x - cs.x) + abs(current_position.y - cs.y);

    for (int i = 1; i < NR_OF_CS; i++) // Start from index 1 since cs is already initialized with index 0
    {
        int current_distance = abs(current_position.x - charging_stations[i].x) + abs(current_position.y - charging_stations[i].y);
        if (min_distance > current_distance)
        {
            cs = charging_stations[i];
            min_distance = current_distance;
        }
    }

    return cs;
}

void print_info()
{
    Serial.println("current position x: " + String(current_position.x) + "  y: " + String(current_position.y));
    Serial.println("destination  x: " + String(destination.x) + "  y: " + String(destination.y));
    Serial.println("battery-level: " + String(battery_level));
    Serial.println("battery-consumption: " + String(battery_consumption));
    Serial.println("range: " + String(range));
    Serial.println("distance: " + String(distance));
    Serial.println("nearest station x: " + String(nearest_charging_station().x) + "  y: " + String(nearest_charging_station().y));
    Serial.println("----------------------------------------------");
    // Serial.println("load: " + String(load) + " kg");
}

String get_pos()
{
    return "4.4";
}

float get_prio()
{
    return priority;
}

position get_curr_pos()
{
    return current_position;
}
int get_id()
{
    return id;
}
int get_load()
{
    return load;
}
int get_place_in_queue()
{
    return place_in_queue;
}
float get_bat_lev()
{
    return battery_level;
}

void set_destination(position dest)
{
    destination = dest;
}

void setNodeList(std::list<u_int32_t> newList)
{
    nodeListState = newList;
}

void broadcastComplete(){
    broadcast_complete = true;
}

void connecting(){
    broadcast_complete = false;
}

void ready_to_charge(bool ready){
    first_in_queue = ready;
}