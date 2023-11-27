#include "state.h"
#include <iostream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED object as an external variable
extern Adafruit_SSD1306 oled;

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
bool charging_complete = false;

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
    Serial.println("current position");
    Serial.print(current_position.x);
    Serial.print(current_position.y);
    Serial.println(" ");
    
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
        if(broadcast_complete){
            next_state = queuing;
            broadcast_complete = false;
        }
        
        break;

    case queuing:
        Serial.println("In state: queuing");
        if(first_in_queue){
            next_state = charging;
            first_in_queue = false;
        }
        break;

    case charging:
        Serial.println("In state: charging");

        next_state = handle_charging();
        break;
    }
    current_state = next_state;

    update_values(); // Updates values
    update_OLED();   // Show valuse
    print_info();    // Terminal printlines

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
state handle_charging()
{
    battery_level += battery_consumption;
    if (battery_level < MAX_BATTERY_LEVEL)
    {
        return charging;
    }
    else
    {
        battery_level = 100;
        return move_to_destination;
    }
}

void initialize_node()

{
    // Seed the random number generator with the current time
    //std::srand(std::time(nullptr));

    load = random(1, MAX_LOAD + 1);
    battery_level = random(MAX_BATTERY_LEVEL + 1);
    battery_consumption = calc_battery_consumption();
    current_position = random_position();
    destination = random_position();
    distance = calc_distance();
    range = calc_range();

    //Function to print ID value to OLED display once
    print_to_OLED_ID(id); 
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

    // Wait-time priority calculation ?

    return priority;
}

position random_position()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    //position p = {std::rand() % 10, std::rand() % 10};
    position p = {random(10), random(10)};
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
    Serial.println("load: " + String(load));
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

void update_OLED(){
    if(current_state == queuing){
        print_to_OLED_battery(place_in_queue);
    }
    else {
        print_not_in_queue_OLED();
        print_to_OLED_range(range);
        print_to_OLED_dist(distance);
        print_to_OLED_consumption(battery_consumption);
        print_to_OLED_battery(battery_level);
    }
}

/* 
All print to OLED methods below starts with clearing the display 
from the old value with the function fillRect(x, y width, height, color ). 
Then the new values prints on the specific coordinates.
 */
void print_to_OLED_queue(int value){
  oled.fillRect(44, 17, 15, 10, BLACK); 
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(44,17);
  oled.println(value);
  oled.display();     
}
void print_to_OLED_ID(int value){
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(76,17);
  oled.println(value);
  oled.display();     
}
void print_to_OLED_range(int value) {
  oled.fillRect(69, 27, 40, 10, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(69,27);
  oled.println(value);
  oled.display();
}
void print_to_OLED_dist(int value) {
  oled.fillRect(69, 37, 40, 10, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(69,37);
  oled.println(value);
  oled.display();
}
void print_to_OLED_consumption(float value) { 
  oled.fillRect(69, 47, 40, 10, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(69,47);
  oled.println(value);
  oled.display();
}
void print_to_OLED_battery(float value){
  oled.fillRect(65, 57, 15, 10, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(65,57);
  oled.println(int(value));
  oled.display();    
}

/*
This method is called when a node is not at a charging station 
to remove its queue value from display. No value is set, display 
is only cleared.
*/
void print_not_in_queue_OLED(){
  oled.fillRect(44, 17, 15, 10, BLACK); 
  oled.display();     
}
void broadcastComplete(){
    broadcast_complete = true;
}
void chargingComplete(){
    charging_complete = true;
}

void connecting(){
    broadcast_complete = false;
}

void ready_to_charge(bool ready){
    first_in_queue = ready;

}