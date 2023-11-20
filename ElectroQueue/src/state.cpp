#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cmath>
#include <vector>


#include "painlessMesh.h"
//#include "common.h"
#include <state.h>
#include <com.h>

/* SET THE INITIAL STATE */
State current_state = move_to_destination;
State next_state = move_to_destination;

String currentZone = ZONE_A_ID;

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
void initialize_charging_stations() {
    Position zone_A;
    zone_A.x = ZONE_A_X;
    zone_A.y = ZONE_A_Y;

    Position zone_B;
    zone_B.x = ZONE_B_X;
    zone_B.y = ZONE_B_Y;

    charging_stations[0] = zone_A;
    charging_stations[1] = zone_B;
    // Add more charging stations if needed
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
    load = std::rand() % MAX_LOAD + 1;
    battery_level = std::rand() % (MAX_BATTERY_LEVEL + 1);
    battery_consumption = calc_battery_consumption();
    current_position = random_position();
    destination = random_position();
    distance = calc_distance();
    range = calc_range();
}

void handle_queuing() {
    nodeList = mesh.getNodeList();

    // 1. Check if all responses in the list have "lower priority"
    Serial.println("in state: handle queueing ");
    // 2. Transition to charging state if all nodes have lower priority
    
}

void handle_connect_and_broadcast(String zoneId) 
{
    // 1. Connect to the mesh network
    meshInit(zoneId, MESH_PASSWORD, MESH_PORT);
    Serial.println("Connected to the mesh network");

    nodeList = mesh.getNodeList();
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

void stateMachine(){
    // Seed the random number generator with the current time
    std::srand(std::time(nullptr));


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
    // Introduce a delay of 1 second
    usleep(2000000); // 1 second = 1,000,000 microseconds

}