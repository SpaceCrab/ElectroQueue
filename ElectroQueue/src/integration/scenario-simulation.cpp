#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cmath>

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

/* STRUCT TO REPRESENT A POSITION IN THE SYSTEM */
struct Position
{
    int x;
    int y;
};

/* GLOBAL VARIABLES */
#define NR_OF_CS 2
#define DIMENSION_LIMIT 10
#define MAX_DISTANCE 19
#define MAX_LOAD 20000
#define MIN_BATTERY_CONSUMPTION 0.3
#define MAX_BATTERY_LEVEL 100

#define   ZONE_A_Y        4
#define   ZONE_A_X        4
#define   ZONE_B_Y        5
#define   ZONE_B_X        5
#define   ZONE_A_ID       "zone A"
#define   ZONE_B_ID       "zone B"

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

Position   cs;




/* DEFINE ALL FUNCTIONS */
Position random_position(void);
Position nearest_charging_station(void);
void initialize_charging_stations(void);
void initialize_node();
void set_node(int lo, float bat_lev, float bat_con, int x_pos, int y_pos, int x_des, int y_des);
bool move(Position dest, int cs);
int calc_distance(void);
float calc_battery_consumption(void);
int calc_range(void);
float calc_priority(void);
void print_info(void);
void handle_move_destination(void);
void handle_move_charging_station(void);
void handle_connect_and_broadcast(void);
void handle_queuing(void);
void handle_charging(void);
void update_values(void);

/* SET THE INITIAL STATE */
State current_state = move_to_destination;
State next_state = move_to_destination;



void state_Machine(){
    // Seed the random number generator with the current time
    std::srand(std::time(nullptr));

    initialize_charging_stations();
    initialize_node();

    while (true)
    {
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
            handle_connect_and_broadcast();
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

void handle_connect_and_broadcast()
{

    /*
    1. meshinit (connect to network)
    2. create list of nodes connected to network
    3. wait (to ensure network connection is stable)
    4. check list
                    if empty -> next-state "charging"
                    if not empty -> next-state "queuing"
    */
    std::cout << "connected to network\n"
              << std::flush;
    std::cout << "list created\n"
              << std::flush;
}

void handle_queuing()
{
    /*
    1. check list
        if all responses (saved) in list == "lower priority" -> next-state = "charging"
        if 1 or more == "higher priority" -> next-state = "queuing"
    */
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

void update_values()
{
    distance = calc_distance();
    range = calc_range();
    print_info();
}
/* creates a random position within the system's dimensions */
Position random_position()
{
    Position p;
    p.x = std::rand() % DIMENSION_LIMIT;
    p.y = std::rand() % DIMENSION_LIMIT;

    return p;
}

/* finds the closest charging station */
Position nearest_charging_station()
{
    int min_distance = abs(current_position.x - ZONE_A_X) + abs(current_position.y - ZONE_A_Y);
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

/* creates random positions for all charging stations */
void initialize_charging_stations()
{    
    Position cs1;
    cs1.x = ZONE_A_X;
    cs1.y = ZONE_A_Y;
    charging_stations[0] = cs1;

    Position cs2;
    cs2.x = ZONE_B_X;
    cs2.y = ZONE_B_Y;
    charging_stations[1] = cs2;
        
        
}

/* sets initial values by randomizing them */
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

/* sets initial values */
void set_node(int lo, float bat_lev, float bat_con, int x_pos, int y_pos, int x_des, int y_des)
{
    load = lo;
    battery_level = bat_lev;
    battery_consumption = calc_battery_consumption();
    Position cur_pos;
    cur_pos.x = x_pos;
    cur_pos.y = y_pos;
    current_position = cur_pos;
    Position des;
    des.x = x_des;
    des.y = y_des;
    destination = des;
    distance = calc_distance();
    range = calc_range();
}

float calc_battery_consumption()
{
    float battery_consumption = (((float)load / MAX_LOAD) * (max_battery_consumption - MIN_BATTERY_CONSUMPTION)) + MIN_BATTERY_CONSUMPTION;

    return battery_consumption;
}

int calc_distance()
{
    distance = 0;
    distance += abs(current_position.x - destination.x);
    distance += abs(current_position.y - destination.y);

    return (int)distance;
}

int calc_range()
{
    range = 0;
    range = std::floor(battery_level / battery_consumption);
    return range;
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
