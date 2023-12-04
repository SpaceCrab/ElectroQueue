#include "painlessMesh.h"
#include <string>
#include <iostream>

#include "state.hpp"
#include "utils.hpp"

// Includes for OLED display
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//mesh network defines 
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

//zone defines 
#define NOT_A_ZONE "NOT A ZONE"

//message defines
#define BROADCAST_PREFIX "BROADCAST"
#define SINGLE_PREFIX "SINGLE"
#define EXIT_PREFIX "EXIT"

// defines for OLED display
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); 

struct message
{
  String msg;
  int id;
};

String currentZoneId;

std::list<message> incomingBuff;
std::list<message> outgoingBuff;

state currentState = move_to_destination;

std::list<response> responseList;
std::list<u_int32_t> nodeList;
std::list<u_int32_t>::iterator nodeListIt;

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void stateCheck();
void handleIncoming();
void handleOutgoing();

Task taskStateCheck(TASK_SECOND * 1, TASK_FOREVER, &stateCheck);
Task taskHandleOutgoing(TASK_SECOND * 1, TASK_FOREVER, &handleOutgoing);
Task TaskHandleIncoming(TASK_SECOND * 1, TASK_FOREVER, &handleIncoming);

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  message incoming;
  incoming.id = from;
  incoming.msg = msg;
  incomingBuff.push_back(incoming);
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  nodeList = mesh.getNodeList();
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  nodeList = mesh.getNodeList();
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void handleIncoming(){
  if(!incomingBuff.empty()){
    message incoming = incomingBuff.front();
    incomingBuff.pop_front();
    
    float ownScore = calc_prio();
    char delimiter = ',';
    int pos = incoming.msg.indexOf(delimiter);
    String scoreStr = incoming.msg.substring(pos + 1);

    String prefix = incoming.msg.substring(0,pos);

    // incoming message is a broadcast, create a response and push it to the outgoing buffer
    // add the node to own responselist, if the score is higher than own as false if lower as true
    if (prefix == BROADCAST_PREFIX && incoming.msg.endsWith(currentZoneId))
    {
      if (pos != -1)
      {
        message outgoing;
        outgoing.id = incoming.id;

        float otherScore = scoreStr.toFloat();

        if (ownScore > otherScore)
        {
          outgoing.msg = "SINGLE,FALSE";
          outgoing.msg += currentZoneId;
          updateOrAddNodeId(incoming.id, true, responseList);
        } 
        if (ownScore < otherScore)
        {
          outgoing.msg = "SINGLE,TRUE";
          outgoing.msg += currentZoneId;
          updateOrAddNodeId(incoming.id, false, responseList);
        } 
        outgoingBuff.push_back(outgoing);
      }
    }
    //the incoming message is a response, add it to the responselist
    else if (prefix == SINGLE_PREFIX && incoming.msg.endsWith(currentZoneId))
    {
      bool response = false;
      if (incoming.msg.endsWith("TRUE"))
        response = true;
      updateOrAddNodeId(incoming.id, response, responseList);
    }
    //the incoming message is a EXIT broadcast from a node leaving the zone 
    else if (prefix == EXIT_PREFIX && incoming.msg.endsWith(currentZoneId)){
      removeFromList(incoming.id, responseList);
    }
     
  }
  TaskHandleIncoming.setInterval(random(1,50));
}

void handleOutgoing(){
  if(outgoingBuff.empty()) return;

  message outgoing = outgoingBuff.front();
  outgoingBuff.pop_front();

  Serial.println(outgoing.msg);
  if(outgoing.msg.startsWith(BROADCAST_PREFIX)||outgoing.msg.startsWith(EXIT_PREFIX)){
    mesh.sendBroadcast(outgoing.msg);
  }
  else if(outgoing.msg.startsWith(SINGLE_PREFIX)){
    mesh.sendSingle(outgoing.id,outgoing.msg);
  }
  else Serial.println("faulty message in outgoing buffer");
  taskHandleOutgoing.setInterval(random(1,50));
}

void createBroadcastMessage(int repetitions, String msg){
  message outgoingBroadcast;
  outgoingBroadcast.id = mesh.getNodeId();
  outgoingBroadcast.msg = msg;

  for(int i = 0; i < repetitions; i++){
    outgoingBuff.push_back(outgoingBroadcast);
  }
}

void createExitMessage(int repetitions){
  message outgoingExit;
  outgoingExit.id = mesh.getNodeId();
  String message = String(EXIT_PREFIX) + "," + String(currentZoneId);
  outgoingExit.msg = message;

  for(int i = 0; i < repetitions; i++){
    outgoingBuff.push_back(outgoingExit);
  }
}

void meshInit(String prefix, String password, int port)
{
  mesh.init(prefix, password, &userScheduler, port);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

bool nodesInNetwork()
{
  nodeList = mesh.getNodeList();
  if (nodeList.empty())
    return false;
  else
    return true;
}

void exitZone()
{
  currentZoneId = NOT_A_ZONE;
}

void enterZone(String zoneID)
{
  currentZoneId = zoneID;
  Serial.printf("entered zone %s", currentZoneId);
}

void stateCheck()
{
  set_id(mesh.getNodeId());
  nodeList = mesh.getNodeList();
  Serial.println("checking state");
  currentState = update_state();
  Serial.println(currentState);

  compareList(nodeList, responseList); // cull disconnected nodes

  String currPos = String(get_curr_pos().x);
  currPos += String(get_curr_pos().y);

  printResponseList(responseList);
  printNodeList(nodeList);
  
  int prio = get_prio_score();
  String broadcast = BROADCAST_PREFIX;
  broadcast += "," + String(prio) + "," + currPos;

  switch (currentState)
  {
  case connect_and_broadcast:
    TaskHandleIncoming.enableIfNot();
    if(!nodeList.empty()){
      enterZone(currPos);
      createBroadcastMessage(1,broadcast);
      broadcastComplete();

      //if(outgoingBuff.empty()) broadcastComplete();
    }
    break;
  case queuing:
    if(incomingBuff.empty()){
      ready_to_charge(allResponseTrue(responseList));
    }
    createBroadcastMessage(1,broadcast);
    Serial.printf("Place in queue %u", queueValue(responseList));
    break;
  case charging:
    break;
  default:
    if(currentZoneId != NOT_A_ZONE){
      responseList.clear();
      createExitMessage(1);
      exitZone();
    }
    TaskHandleIncoming.disable();
    break;
  }
}
// Needed for painless library

void setup()
{
  Serial.begin(115200);

  randomSeed(analogRead(2));
 //Setup part for OLED display

// initialize OLED display with address 0x3C for 128x64
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("SSD1306 allocation failed"));
      while (true);
    }
  delay(2000);                    // wait for initializing
  oled.clearDisplay();            // clear display

  oled.setTextSize(1);         
  oled.setTextColor(WHITE);      
  oled.setCursor(32, 17);         
  oled.println("Q:");             // Nodes queue 
  oled.setCursor(60, 17);         
  oled.println("ID:");           // Nodes ID 
  oled.setCursor(32, 27);
  oled.println("Range:");         // Range
  oled.setCursor(32, 37);         
  oled.println("Dist:");          // Distance 
  oled.setCursor(32, 47);         
  oled.println("Cons:");          // Battery consumtion  
  oled.setCursor(32, 57);         
  oled.println("Batt:");          // Battery levet
  oled.setCursor(86, 57);         
  oled.println('%');
  oled.display();                 // show on OLED

  initialize_charging_stations();
  Serial.println("statemachine init");
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  meshInit("ElectroQueue", MESH_PASSWORD, MESH_PORT);

  Serial.println("adding state tasks");
  userScheduler.addTask(taskStateCheck);
  Serial.println("setting interval");
  taskStateCheck.setInterval(900);
  Serial.println("statemachine enable");
  taskStateCheck.enable();

  Serial.println("Adding comm tasks");
  userScheduler.addTask(taskHandleOutgoing);
  userScheduler.addTask(TaskHandleIncoming);

  Serial.println("enabling comm tasks");
  taskHandleOutgoing.enable();
  TaskHandleIncoming.enable();

  // Must be here to work with OLED
  Serial.println("node initializing");
  set_id(mesh.getNodeId());
  initialize_node();
  initialize_charging_stations();

  exitZone();

  Serial.println("init done");

}

void loop()
{
  mesh.update();
}
