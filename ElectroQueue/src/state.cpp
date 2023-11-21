#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cmath>
#include <vector>

#include <painlessMesh.h>

#include <com.h>

/*Macros */
#define NR_OF_CS 2
#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100

std::list<u_int32_t> nodeList;

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

/* SET THE INITIAL STATE */
State current_state = move_to_destination;
State next_state = move_to_destination;

String currentZone = ZONE_A_ID;

Position random_position()
{
    Position p;
    p.x = std::rand() % DIMENSION_LIMIT;
    p.y = std::rand() % DIMENSION_LIMIT;
    return p;
}

Position nearest_charging_station()
{
    Position cs = charging_stations[0];
    int min_distance = abs(current_position.x - charging_stations[0].x) + abs(current_position.y - charging_stations[0].y);

    for (int i = 0; i < NR_OF_CS; i++)
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

void handle_move_destination()
{
    if (range < distance)
    {
        next_state = move_to_nearest_charging_station;
    }
    else
    {
        if (move(destination, 0))
        {

            next_state = assign_new_destination;
        }
        else
        {
            next_state = move_to_destination;
        }
    }
}

void handle_move_charging_station()
{
    if (move(nearest_charging_station(), 1))
    {
        next_state = connect_and_broadcast;
    }
    else
    {
        next_state = move_to_nearest_charging_station;
    }
}

void handle_charging()
{
    /* if a connecting node broadcasts a "alert" flag
    jump back to communicate priority
    */

    float bat_for_des = calc_distance() * battery_consumption;
    // a node will charge so that it can reach its destination, plus an addition of 20 percent of that
    battery_level += (bat_for_des * 1.2);
    // but it will never be charged more than 100 %
    if (battery_level > MAX_BATTERY_LEVEL)
    {
        battery_level = MAX_BATTERY_LEVEL;
    }
    next_state = move_to_destination;
}

bool move(Position dest, int cs)
{
    bool x = false;
    bool y = false;

    if (dest.x == current_position.x)
    {
        x = true;
    }
    else
    {
        if (dest.x > current_position.x)
        {
            current_position.x += 1;
        }
        else
        {
            current_position.x -= 1;
        }
        if (cs == 0)
        {
            battery_level -= battery_consumption;
        }
        if (dest.x == current_position.x)
        {
            x = true;
        }
    }

    if (dest.y == current_position.y)
    {
        y = true;
    }
    else
    {
        if (dest.y > current_position.y)
        {
            current_position.y += 1;
        }
        else
        {
            current_position.y -= 1;
        }
        if (cs == 0)
        {
            battery_level -= battery_consumption;
        }
        if (dest.y == current_position.y)
        {
            y = true;
        }
    }

    if (x && y)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void print_info()
{
    std::cout << "current position x: " << current_position.x << "  y: " << current_position.y << "\n"
              << std::flush;
    std::cout << "destination  x: " << destination.x << "  y: " << destination.y << "\n"
              << std::flush;
    std::cout << "battery-level: " << battery_level << "\n"
              << std::flush;
    std::cout << "battery-consumption: " << battery_consumption << "\n"
              << std::flush;
    std::cout << "range: " << range << "\n"
              << std::flush;
    std::cout << "distance: " << distance << "\n"
              << std::flush;
    std::cout << "nearest station x: " << nearest_charging_station().x << "  y: " << nearest_charging_station().y << "\n"
              << std::flush;
    std::cout << "----------------------------------------------\n"
              << std::flush;
    // std::cout << "load: " << load << " kg \n";
}

void initialize_charging_stations() {
    Position zone_A;
    zone_A.x = ZONE_A_X;
    zone_A.y = ZONE_A_Y;

    Position zone_B;
    zone_B.x = ZONE_B_X;
    zone_B.y = ZONE_B_Y;

    charging_stations[0] = zone_A;
    charging_stations[1] = zone_B;
}

void update_values()
{
    distance = calc_distance();
    range = calc_range();
    print_info();
}

int calc_range()
{
    range = 0;
    range = std::floor(battery_level / battery_consumption);
    return range;
}

int calc_distance()
{
    distance = 0;
    distance += abs(current_position.x - destination.x);
    distance += abs(current_position.y - destination.y);

    return (int)distance;
}

float calc_battery_consumption()
{
    float battery_consumption = (((float)load / MAX_LOAD) * (max_battery_consumption - MIN_BATTERY_CONSUMPTION)) + MIN_BATTERY_CONSUMPTION;

    return battery_consumption;
}

void initialize_node()
{
    std::srand(std::time(nullptr));
    load = std::rand() % MAX_LOAD + 1;
    battery_level = std::rand() % (MAX_BATTERY_LEVEL + 1);
    battery_consumption = calc_battery_consumption();
    current_position = random_position();
    destination = random_position();
    distance = calc_distance();
    range = calc_range();
}

void handle_queuing() {
    nodeList = getNodeList();

    // 1. Check if all responses in the list have "lower priority"
    Serial.println("in state: handle queueing ");
    // 2. Transition to charging state if all nodes have lower priority
    
}

void handle_connect_and_broadcast(String zoneId) 
{
    // 1. Connect to the mesh network
    enterZone(currentZone);
    Serial.println("Connected to the mesh network");

    nodeList = getNodeList();
    // 4. Check the list of connected nodes
    
    if (nodeList.size() == 0) {
        // If the list is empty, go to the next state "charging"
        next_state = charging;
        Serial.println("No nodes in the network. Going to charging state.");
    } else {
        // If not empty, go to the next state "queuing"
        next_state = queuing;
        Serial.println("Some nodes connected. Going to queuing state.");
    }
}

float calc_priority()
{
    float priority = 0;

    if (battery_level >= 85)
    {
        priority += (battery_level / MAX_BATTERY_LEVEL) * 3;
    }
    else if (battery_level <= 15)
    {
        priority += ((MAX_BATTERY_LEVEL - battery_level) / MAX_BATTERY_LEVEL) * 3;
    }
    else
    {
        priority += ((MAX_BATTERY_LEVEL - battery_level) / MAX_BATTERY_LEVEL) * 2;
    }

    if (range > distance)
    {
        priority += (battery_consumption / max_battery_consumption) * 1;
    }
    else
    {
        if (range > (distance / 2))
        {
            priority += (battery_consumption / max_battery_consumption) * 2;
        }
        else
        {
            priority += ((distance - range) / distance) * 2;
            priority += (battery_consumption / max_battery_consumption) * 1;
        }
    }

    return priority;
}

void stateMachine(){

    switch (current_state)
    {
    case assign_new_destination:
        std::cout << "in state: assign new destination\n"
                    << std::flush;
        destination = random_position();
        next_state = move_to_destination;
        break;

    case move_to_destination:
        std::cout << "in state: move to destination\n"
                    << std::flush;
        handle_move_destination();
        break;

    case move_to_nearest_charging_station:
        std::cout << "in state: move to nearest charging station\n"
                    << std::flush;
        handle_move_charging_station();
        break;

    case connect_and_broadcast:
        std::cout << "in state: connect and broadcast\n"
                    << std::flush;
        
        handle_connect_and_broadcast(currentZone);
        next_state = charging;
        break;

    case queuing:
        std::cout << "in state: queuing\n"
                    << std::flush;
        handle_queuing();
        next_state = charging;
        break;

    case charging:
        std::cout << "in state: charging\n"
                    << std::flush;
        handle_charging();
        break;
    }
    current_state = next_state;
    update_values();
}