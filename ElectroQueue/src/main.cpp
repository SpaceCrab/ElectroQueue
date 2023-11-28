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

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555
#define ZONE_A_Y 4
#define ZONE_A_X 4
#define ZONE_B_Y 5
#define ZONE_B_x 5
#define ZONE_A_ID "zone A"
#define ZONE_B_ID "zone B"
#define BROADCAST_PREFIX "BROADCAST"
#define SINGLE_PREFIX "SINGLE"
// defines for OLED display
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); 

int posY = 0;
int posX = 0;
int queuePos = 0;

String currentMsg;
u_int32_t currentTarget;
String zoneId;

state currentState = move_to_destination;

std::list<response> responseList;
std::list<u_int32_t> nodeList;
std::list<u_int32_t>::iterator nodeListIt;

String prevZoneID;

bool networkstate = false;

uint32_t testMessagesSent = 0;

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void sendMessage(); // Prototype so PlatformIO doesn't complain
void sendBroadcast();
void sendSingleMsg();
void stateCheck();

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
Task taskStateCheck(TASK_SECOND * 1, TASK_FOREVER, &stateCheck);
Task taskSendBroadcast(TASK_SECOND * 1, TASK_FOREVER, &sendBroadcast);
Task taskSendSingle(TASK_SECOND * 1, TASK_FOREVER, &sendSingleMsg);

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  /*when the recieved msg is a broadcast with prio score, compare the score.
      if the score is lower than the own score: respond with false and add it to the responselist
      if the score is higher than the own score: respond with true and add it to the responselist
  */
  if (currentState != queuing)
    return;
  if (msg.startsWith(BROADCAST_PREFIX))
  {
    Serial.println("Broadcast message recieved");
    float ownScore = calc_prio();
    char delimiter = ',';

    int pos = msg.indexOf(delimiter);
    if (pos != -1)
    {
      String scoreStr = msg.substring(pos + 1);
      int otherScore = scoreStr.toInt();

      if (ownScore > otherScore)
      {
        currentMsg = "SINGLE,FALSE";
        currentTarget = from;
        taskSendSingle.setIterations(3);
        taskSendSingle.enable();
        updateOrAddNodeId(from, true, responseList);
      } // send response with false;

      if (ownScore < otherScore)
      {
        currentMsg = "SINGLE,TRUE";
        currentTarget = from;
        taskSendSingle.setIterations(3);
        taskSendSingle.enable();
        updateOrAddNodeId(from, false, responseList);
      } // send response with true;
    }
  }
  /*when the recieved msg is a response: store the response in the responselist
   */
  else if (msg.startsWith(SINGLE_PREFIX))
  {
    bool response = false;
    if (msg.endsWith("TRUE"))
      response = true;
    updateOrAddNodeId(from, response, responseList);
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  nodeList = mesh.getNodeList();
  setNodeList(nodeList);
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void meshInit(String prefix, String password, int port)
{
  if (!networkstate)
  {
    mesh.init(prefix, password, &userScheduler, port);
    networkstate = true;
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  }
}
// User stub

void sendBroadcast()
{
  mesh.sendBroadcast(currentMsg);
  Serial.println("sent broadcast message");
}

void sendSingleMsg()
{
  mesh.sendSingle(currentTarget, currentMsg);
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
  if (networkstate)
  {
    mesh.stop();
    nodeList = mesh.getNodeList();
    networkstate = false;
    Serial.println("left network");
  }
}

void enterZone(String zoneID)
{
  if (!networkstate)
  {
    Serial.println("entered zone ");
    Serial.println(zoneID);

    Serial.println("starting network.....");
    meshInit(zoneID, MESH_PASSWORD, MESH_PORT);
    networkstate = true;
  }
}

void sendMessage()
{
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  testMessagesSent++;
  Serial.println("sent message");

  // test code

  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void stateCheck()
{
  Serial.println("checking state");
  compareList(nodeList, responseList);
  currentState = update_state();
  Serial.println(currentState);
  printResponseList(responseList);
  printNodeList(nodeList);

  switch (currentState)
  {
  case connect_and_broadcast:
    // returns a struct -> position = {x,y}
    zoneId = get_curr_pos().x;
    zoneId += get_curr_pos().y;
    Serial.println("zone id");
    Serial.println(zoneId);
    
    connecting();
    enterZone(zoneId);
    if (!nodeList.empty())
    {
      String prio = String(calc_prio());
      currentMsg = "BROADCAST," + prio; // replace with real score from state machine
      taskSendBroadcast.setIterations(4);
      taskSendBroadcast.enable();
      broadcastComplete();
    }
    else
      Serial.println("no other nodes");
      //broadcastComplete();

    break;
  case queuing:
    ready_to_charge(allResponseTrue(responseList));
    break;
  case charging:
    break;
  default:
    exitZone();

    break;
  }
}
// Needed for painless library

void setup()
{
  Serial.begin(115200);

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
  meshInit(ZONE_A_ID, MESH_PASSWORD, MESH_PORT);
  networkstate = true;

  initialize_node();

  Serial.println("adding tasks");
  userScheduler.addTask(taskStateCheck);
  taskStateCheck.setInterval(500);
  Serial.println("statemachine enable");
  taskStateCheck.enable();
  Serial.println("init done");

  userScheduler.addTask(taskSendSingle);

  userScheduler.addTask(taskSendBroadcast);

  Serial.println("init complete");

  
  // Must be here to work with OLED
  initialize_node();
  initialize_charging_stations();


  exitZone();

}

void loop()
{
  mesh.update();
}
