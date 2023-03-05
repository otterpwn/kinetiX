/*
  Part of KinetiX project
  KinetiX ESP Client
  
  Author: Riccardo "Cinderz" Miani ʕ •ᴥ•ʔ
  Licence: 
*/

// include library for JSON
#include <Arduino_JSON.h>

// include required libraries for MPU-6050
#include <MPU6050.h>
#include <Wire.h>

// include required libraries for Painless Mesh protocol
#include <painlessMesh.h>

// initialize accelerometer object
MPU6050 mpu;

// initialize scheduler and mesh objects
Scheduler userScheduler;
painlessMesh  mesh;

// define strings for readings
String readings;

// define node number
int nodeNumber = 1;

// define variables for max coords calculation
double maxX = 0;
double maxY = 0;
double maxZ = 0;

// define variables for max coords calculation
double minX = 0;
double minY = 0;
double minZ = 0;

// define credentials and port for access point
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// define functions used by mesh
// functions receivedCallback, newConnectionCallback, newConnectionCallback and nodeTimeAdjustedCallback are required for painless mesh library

// user stubs
void sendMessage();
String getReadings();

Task taskSendMessage(TASK_SECOND * 0.5, TASK_FOREVER, &sendMessage);

String getReadings() {
  JSONVar jsonReadings;
  
  // get accelerometer events
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &ax, &gx, &gy, &gz);

  jsonReadings["node"] = nodeNumber;
  jsonReadings["XAXIS"] = String(ax);
  jsonReadings["YAXIS"] = String(ay);
  jsonReadings["ZAXIS"] = String(az);
  readings = JSON.stringify(jsonReadings);

  Serial.println(ax);
  
  return readings;
}

void sendMessage() {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
  
  // send message every second
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 1 ));
}

void initmpu() {
  // try to initialize accelerometer
  Wire.begin();
  mpu.initialize();

  Serial.println (mpu.testConnection() ? "MPU6050 Found!" : "No MPU6050 Found!"); 

  // set accelerometer measurement range
  // options are
  // 1. MPU6050_RANGE_2_G
  // 2. MPU6050_RANGE_4_G
  // 3. MPU6050_RANGE_8_G
  // 4. MPU6050_RANGE_16_G
  //mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

  // set gyroscope measurement range
  // options are
  // 1. MPU6050_RANGE_250_DEG
  // 2. MPU6050_RANGE_500_DEG
  // 3. MPU6050_RANGE_1000_DEG
  // 4. MPU6050_RANGE_2000_DEG
  //mpu.setGyroRange(MPU6050_RANGE_2000_DEG);

  // set filter bandwitdth
  // options are
  // 1. MPU6050_BAND_260_HZ
  // 2. MPU6050_BAND_184_HZ
  // 3. MPU6050_BAND_94_HZ
  // 4. MPU6050_BAND_44_HZ
  // 5. MPU6050_BAND_21_HZ
  // 6. MPU6050_BAND_10_HZ
  // 7. MPU6050_BAND_5_HZ
  //mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar readingsObj = JSON.parse(msg.c_str());
  
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

  // initialize accelerometer
  initmpu();  

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