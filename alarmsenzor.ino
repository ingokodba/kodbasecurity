#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

int pirPin = 5;                 // PIR Out pin 
int pirStat = 0;                   // PIR status
int pirVal = 0;                   // PIR status

const int buzzer = 4;
const uint16_t RECV_PINa = 2;         //Deklaracija za IR

IRrecv irrecv(RECV_PINa);

decode_results results;

void setup() {

  Serial.begin(115200);
  pinMode(pirPin, INPUT);     
  Serial.setDebugOutput(true);

  pinMode(buzzer, INPUT);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ssid", "password");

  irrecv.enableIRIn();  // Start the receiver
}

void buzzaj(){
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  for(int i = 0; i < 1000; i++){
    pinMode(buzzer, INPUT);
    delayMicroseconds(300);
    pinMode(buzzer, OUTPUT);
    delayMicroseconds(300);
  }
}

void sendIngoRequest(int broj){
  String saljem = "";
  switch(broj){
    case 0:
      saljem = "http://192.168.2.15/on";
      break;
    case 1:
      saljem = "http://192.168.2.15/off";
      break;
    case 2:
      saljem = "http://192.168.2.15/trigger";
      break;
    case 3:
      saljem = "http://192.168.2.15/siren";
      break;
  }
  Serial.print("saljem ");
  Serial.println(saljem);

  int pokusaj = 0;
  bool poslano = false;
  while(!poslano && pokusaj < 10){
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED)) {
  
      WiFiClient client;
      HTTPClient http;
  
      Serial.print("[HTTP] begin...\n");
      if (http.begin(client, saljem)) {  // HTTP
        
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            poslano = true;
            String payload = http.getString();
            Serial.println(payload);
            buzzaj();
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    }
    pokusaj++;
    delay(500);
  }
}


void loop() {
  int pirVal = digitalRead(pirPin); 
  if (pirVal == HIGH && pirStat == LOW) { // if motion detected
    Serial.println("Motion on.");
    sendIngoRequest(2);    
    pirStat = HIGH;
  }
  if (pirVal == LOW && pirStat == HIGH) { // if no motion
    Serial.println("Motion off.");
    pirStat = LOW;
  }

  bool found = false;

  if (irrecv.decode(&results)) {
    
    Serial.println(results.value, HEX);
    irrecv.resume(); 

    switch(results.value){      
      case 0xFFA25D:
        Serial.println("on");
        sendIngoRequest(0);
        found = true;
        break;
      case 0xFF629D:
        Serial.println("off");
        sendIngoRequest(1);
        found = true;
        break;
      case 0xFFE21D:
        Serial.println("siren");
        sendIngoRequest(3);
        found = true;
        break;
      }   
    if(found){
      buzzaj();
      delay(500);
    }
  }
}
