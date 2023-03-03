#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"

// create AsyncWebServer object on port 80
AsyncWebServer server(80);

// set access point credentials
const char* ssid = "kinetiXServer";
const char* password = "kinetix";

// degfine pins for joystick
#define VRX_PIN 0 // vrx pin is digital0
#define VRY_PIN 1 // vry pin is digital1

// define functions to read values
String readX() {
  return String(digitalRead(VRX_PIN));
}

String readY() {
  return String(digitalRead(VRY_PIN));
}

void setup() {
  // initialize serial port
  Serial.begin(115200);
  Serial.println();

  // initialize access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  // get and print IP address
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/vrx", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readX().c_str());
  });

  server.on("/vry", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readY().c_str());
  });

  // start server
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
