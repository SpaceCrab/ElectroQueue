#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cmath>
#include <vector>


#include "painlessMesh.h"



void stateMachine(){
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

/*void handle_connect_and_broadcast()
{

    /*
    1. meshinit (connect to network)
    2. create list of nodes connected to network
    3. wait (to ensure network connection is stable)
    4. check list
                if empty -> next-state "charging"
                if not empty -> next-state "queuing"
    
    std::cout << "connected to network\n"
              << std::flush;
    std::cout << "list created\n"
              << std::flush;
}*/

/*void handle_connect_and_broadcast() 
{
    // 1. Connect to the mesh network
    meshInit(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    Serial.println("Connected to the mesh network");


    // 2. Create a list of nodes connected to the network
    NodeList nodes = mesh.getNodeList();
    std::vector<Node> connectedNodes;  


    // 3. Wait to ensure network connection is stable
    delay(2000); 

    // 4. Check the list of connected nodes
    //if (connectedNodes.empty()) {
    if (nodes.size() == 0) {
        // If the list is empty, go to the next state "charging"
        next_state = charging;
        Serial.println("No nodes in the network. Going to charging state.");
    } else {
        // If not empty, go to the next state "queuing"
        next_state = queuing;
        Serial.println("Some nodes connected. Going to queuing state.");
    }
}*/


/*void handle_queuing()
{
    /*
    1. check list
        if all responses (saved) in list == "lower priority" -> next-state = "charging"
        if 1 or more == "higher priority" -> next-state = "queuing"
    
}*/

/*void handle_queuing() {
    NodeList nodes = mesh.getNodeList();

    // 1. Check if all responses in the list have "lower priority"
    bool allLowerPriority = true;
    for (const auto& node : nodes) {
        // Assuming nodes have a priority value, adjust this condition based on your node structure
        if (node.getPriority() > LOWER_PRIORITY_THRESHOLD) {
            allLowerPriority = false;
            break;
        }
    }

    // 2. Transition to charging state if all nodes have lower priority
    if (allLowerPriority) {
        next_state = charging;
        Serial.println("All nodes have lower priority. Going to charging state.");
    } else {
        // 3. If any node has "higher priority," stay in the queuing state
        Serial.println("Some nodes have higher priority. Remaining in queuing state.");
    }
}*/


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

/* creates random positions for all charging stations */
/*void initialize_charging_stations()
{
    for (int i = 0; i < NR_OF_CS; i++)
    {
        Position cs = random_position();
        charging_stations[i] = cs;
    }
}*/

/* CREATE CHARGING STATIONS BASED ON ZONE DEFINITIONS */
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
/*void set_node(int lo, float bat_lev, float bat_con, int x_pos, int y_pos, int x_des, int y_des)
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
}*/

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

void exitZone(){
  if(networkstate){
    taskSendMessage.disable();
    mesh.stop();
    networkstate = false;
    Serial.println("left network");
  }
}

void enterZone(String zoneID){
  if(!networkstate)
  {
    Serial.println("entered zone ");
    Serial.println(zoneID);

    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    taskSendMessage.enableIfNot();
    networkstate = true;
  }
}
// test function to set a random position
void updatePosition()
{
  Serial.println("updated position ");
   
  //chooses a random position 
  Position p;
  p.x = random(4,6);
  p.y = random(4,6);

  Serial.print(p.x);
  Serial.println(p.x);

  String str = "free memory: ";
  str += ESP.getFreeHeap();
  Serial.println(str);

  //check which zone the nodes is in
  //TODO: make this into a switch or some type of handler for multiple zones
  if(p.x == ZONE_A_Y && p.x == ZONE_A_X ) {
    Serial.println("in zone A");
    if(prevZoneID != ZONE_A_ID) { //check for zone change 
      exitZone();                 //leave the old network
      enterZone(ZONE_A_ID);       // create or enter the new network
      Serial.println("entered zone A from ");
      Serial.print(prevZoneID);
    }
    else{
      Serial.println("same zone, nothing changed ");
    }
    prevZoneID = ZONE_A_ID;
  }
  else if (p.x == ZONE_B_Y && p.x == ZONE_B_X ){
    Serial.println("in zone B");
    if(prevZoneID != ZONE_B_ID){
      exitZone();
      enterZone(ZONE_B_ID);
      Serial.println("Entered zone B from ");
      Serial.print(prevZoneID);
    }
    else{
      Serial.println("same zone, nothing changed ");
    }
    prevZoneID = ZONE_B_ID;
  }
  else{
    prevZoneID = "not a zone";
    Serial.println("leaving zone");
    exitZone();
  }
}

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  testMessagesSent++;
  Serial.println("sent message");

  // test code 
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
// meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);
//  mesh.onReceive(&receivedCallback);
//  mesh.onNewConnection(&newConnectionCallback);
//  mesh.onChangedConnections(&changedConnectionCallback);
//  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  Serial.println("creating scheduler tasks ");
  Serial.println("taskSendmessage");
  userScheduler.addTask( taskSendMessage );

  Serial.println("taskUpdatePosition");
  userScheduler.addTask(taskUpdateposition);
  taskUpdateposition.setInterval(30000);

  Serial.println("taskUpdatePosition enable");
  taskUpdateposition.enable();

  Serial.println("creating scheduler tasks ");
  Serial.println("taskStateMachine");
  userScheduler.addTask(taskStateMachine);
  taskStateMachine.enable();
  taskStateMachine.setInterval(10000);

  //Serial.println("taskSendMessage enable");
  //taskSendMessage.enable();

  Serial.println("init complete");
}

void loop() {
  mesh.update();
}