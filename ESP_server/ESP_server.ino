/*
  Part of KinetiX project
  KinetiX ESP Server
  
  Author: Riccardo "Cinderz" Miani ʕ •ᴥ•ʔ
  Licence: 
*/

// include library for JSON
#include <Arduino_JSON.h>

// include required libraries for Painless Mesh protocol
#include <painlessMesh.h>

// include libraries for servo control
#include <Servo.h>

// initialize scheduler and mesh objects
Scheduler userScheduler;
painlessMesh  mesh;

// define strings for readings
String readings;

// define servo object
Servo servoX;

// define node number
int nodeNumber = 2;

// define credentials and port for access point
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// define functions used by mesh
// functions receivedCallback, newConnectionCallback, newConnectionCallback and nodeTimeAdjustedCallback are required for painless mesh library

// user stubs
void sendMessage();
String getReadings();

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendMessage() {
  String msg = "Whopper, Whopper, Whopper, Whopper";
  mesh.sendBroadcast(msg);

  // send message every second
  taskSendMessage.setInterval(random( TASK_SECOND * 1, TASK_SECOND * 1 ));
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  //JSONVar readingsObj = JSON.parse(msg.c_str());
  JSONVar readingsObj = JSON.parse(msg);

  int node = readingsObj["node"];
  String sXAXIS = readingsObj["XAXIS"];
  String sYAXIS = readingsObj["YAXIS"];
  String sZAXIS = readingsObj["ZAXIS"];

  int16_t XAXIS = sXAXIS.toInt();
  int16_t YAXIS = sYAXIS.toInt();
  int16_t ZAXIS = sZAXIS.toInt();

  /*
  Serial.print("Node: ");
  Serial.println(node);
  Serial.print("X: ");
  Serial.println(XAXIS);
  Serial.print("Y: ");
  Serial.println(YAXIS);
  Serial.print("Z: ");
  Serial.println(ZAXIS);
  */

  // map coord values to servo motion
  // MPU6050 library gives values in range of [-17000, 17000]
  XAXIS = map(XAXIS, -17000, 17000, 0, 180);

  Serial.println(XAXIS);

  // write values to servos
  servoX.write(XAXIS);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup(void) {
  // initialize serial monitor with baud-rate of 115200
  Serial.begin(115200);

  // delay if serial monitor does not start
  while (!Serial) {
    delay(10);
  }

  // attach servoX to GPIO5 or D1
  servoX.attach(5);

  // reset servo position
  servoX.write(0);

  // set startup messages for mesh
  // options are
  // 1. ERROR
  // 2. MESH_STATUS
  // 3. CONNECTION
  // 4. SYNC
  // 5. COMMUNICATION
  // 6. GENERAL
  // 7. MSG_TYPES
  // 8. REMOTE
  // 9. STARTUP  
  mesh.setDebugMsgTypes(ERROR | STARTUP);

  // init mesh
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // define behavior for events
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // execute task
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // update mesh
  mesh.update();
}