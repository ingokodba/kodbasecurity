#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef STASSID
#define STASSID "ssid"
#define STAPSK  "password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int relay = 12; // relay pin to control NEKA siren
bool sirenWaitingOn = false;
bool alarm = false;
bool siren = false;
unsigned long timeToStart;
unsigned long timeToSiren;
unsigned long timeToOff;

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void sirenOff(){
  pinMode(relay, INPUT);
  siren = false;
  sirenWaitingOn = false;
  Serial.println("siren off");
}

void sirenOn(){
  pinMode(relay, OUTPUT);
  siren = true;
  timeToOff = millis()+2*60000;
  Serial.println("siren on");
}

void setup(void) {
  Serial.begin(115200);
  sirenOff();
  timeToStart = millis();
  timeToOff = timeToStart;
  timeToSiren = timeToStart;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/trigger", []() {
    if(alarm && !siren && millis() > timeToStart){
      timeToSiren = millis()+20*1000;
      sirenWaitingOn = true;
      server.send(200, "text/plain", "trigger");
    } else {
      server.send(200, "text/plain", "ok");
    }
  });

  server.on("/off", []() {
    sirenOff();
    alarm = false;    
    server.send(200, "text/plain", "alarm off");
    Serial.println("alarm off");
  });

  server.on("/on", []() {
    sirenOff();
    alarm = true;
    timeToStart = millis()+30*1000;
    server.send(200, "text/plain", "alarm on");
    Serial.println("alarm on");
  });

  server.on("/siren", []() {
    sirenOn();
    server.send(200, "text/plain", "siren turned on now");
  });

  server.on("/hello", []() {
    server.send(200, "text/plain", "hello back");
    Serial.println("saying hello:)");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  if(siren == true && timeToOff < millis()){
    sirenOff();
    Serial.println("loop siren off after a while");
  }
  if(alarm && sirenWaitingOn && millis() > timeToSiren){
    sirenOn();
    Serial.println("loop siren on trigger after certain delay");
  }
}
