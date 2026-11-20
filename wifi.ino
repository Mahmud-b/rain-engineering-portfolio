#include <ESP8266WiFi.h>

// Your phone's hotspot details
const char* ssid = "King Biceps";
const char* password = "Mubarak@4484";

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  Serial.println("Connecting to hotspot");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected successfully!");

  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

} 
