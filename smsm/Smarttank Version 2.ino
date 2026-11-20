
#include <SoftwareSerial.h>

// ESP8266 communication: Uno RX = D10, TX = D11
SoftwareSerial espLink(10, 11);

// Ultrasonic sensor
#define TRIG_PIN 6
#define ECHO_PIN 7

// Relay connections
#define PUMP_RELAY 4
#define NEGATIVE_RELAY 5
#define POSITIVE_RELAY 9

// Source sensors
#define LDR_PIN A0
#define NEPA_PIN A1

// Source thresholds
const int NEPA_THRESHOLD = 400;
const int SOLAR_THRESHOLD = 500;

// Percentage-based pump thresholds
const int START_LEVEL = 40;
const int STOP_LEVEL = 91;

// Active LOW relays
#define RELAY_ON LOW
#define RELAY_OFF HIGH

float tankHeight = 0;
bool tankConfigured = false;

bool pumpOn = false;
bool waterDemand = false;

enum Source {
  SOURCE_OFF,
  SOURCE_NEPA,
  SOURCE_SOLAR
};

Source currentSource = SOURCE_OFF;

void readESPCommands() {
  static String command = "";

  while (espLink.available()) {
    char c = espLink.read();

    if (c == '\n') {
      command.trim();

      if (command == "R") {
        // ESP restarted: require tank setup again
        tankConfigured = false;
        tankHeight = 0;
        waterDemand = false;
        pumpOn = false;

        digitalWrite(PUMP_RELAY, RELAY_OFF);

        espLink.println("ACK,RESET");
        Serial.println("ESP RESET: Tank setup required");
      }

      else if (command.startsWith("H,")) {
        float newHeight = command.substring(2).toFloat();

        if (newHeight >= 5.0 && newHeight <= 200.0) {
          tankHeight = newHeight;
          tankConfigured = true;

          waterDemand = false;
          pumpOn = false;

          digitalWrite(PUMP_RELAY, RELAY_OFF);

          espLink.println("ACK,HEIGHT");

          Serial.print("Tank configured: ");
          Serial.print(tankHeight);
          Serial.println(" cm");
        }
      }

      command = "";
    }

    else if (c != '\r' && command.length() < 40) {
      command += c;
    }
  }
}

void selectSource(Source desiredSource) {
  if (desiredSource == currentSource) return;

  // Stop pump before changing source
  digitalWrite(PUMP_RELAY, RELAY_OFF);
  delay(300);

  if (desiredSource == SOURCE_NEPA) {
    digitalWrite(POSITIVE_RELAY, RELAY_OFF);
    digitalWrite(NEGATIVE_RELAY, RELAY_OFF);
  }

  else if (desiredSource == SOURCE_SOLAR) {
    digitalWrite(POSITIVE_RELAY, RELAY_ON);
    digitalWrite(NEGATIVE_RELAY, RELAY_ON);
  }

  delay(300);
  currentSource = desiredSource;
}

void setup() {
  Serial.begin(9600);
  espLink.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(NEGATIVE_RELAY, OUTPUT);
  pinMode(POSITIVE_RELAY, OUTPUT);

  pinMode(LDR_PIN, INPUT);
  pinMode(NEPA_PIN, INPUT);

  digitalWrite(PUMP_RELAY, RELAY_OFF);
  digitalWrite(NEGATIVE_RELAY, RELAY_OFF);
  digitalWrite(POSITIVE_RELAY, RELAY_OFF);

  Serial.println("SMARTTANK STARTED");
  Serial.println("Waiting for tank configuration...");
}

void loop() {
  // 1. Receive tank setup commands from ESP
  readESPCommands();

  // 2. Read power sources
  int nepaReading = analogRead(NEPA_PIN);
  int lightReading = analogRead(LDR_PIN);

  bool nepaAvailable = nepaReading >= NEPA_THRESHOLD;
  bool solarAvailable = lightReading >= SOLAR_THRESHOLD;

  Source desiredSource;

  if (nepaAvailable) {
    desiredSource = SOURCE_NEPA;
  }
  else if (solarAvailable) {
    desiredSource = SOURCE_SOLAR;
  }
  else {
    desiredSource = SOURCE_OFF;
  }

  // 3. Measure water distance
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  float distance = -1;
  float waterHeight = 0;
  int percentage = 0;

  bool validReading = false;

  if (tankConfigured && duration > 0) {
    distance = duration * 0.0343 / 2.0;

    // Allow a small measurement margin beyond tank height
    if (distance >= 1.5 && distance <= tankHeight + 5.0) {
      validReading = true;

      waterHeight = tankHeight - distance;
      waterHeight = constrain(waterHeight, 0, tankHeight);

      percentage = (waterHeight / tankHeight) * 100;
      percentage = constrain(percentage, 0, 100);
    }
  }

  // 4. Percentage-based pump control
  if (!validReading) {
    waterDemand = false;
  }
  else {
    if (percentage <= START_LEVEL) {
      waterDemand = true;
    }

    if (percentage >= STOP_LEVEL) {
      waterDemand = false;
    }
  }

  // 5. Select power source
  selectSource(desiredSource);

  // 6. Pump control
  pumpOn = tankConfigured &&
           validReading &&
           waterDemand &&
           (desiredSource != SOURCE_OFF);

  digitalWrite(
    PUMP_RELAY,
    pumpOn ? RELAY_ON : RELAY_OFF
  );

  // 7. Send readings and configuration status to ESP
  espLink.print(distance);
  espLink.print(",");
  espLink.print(percentage);
  espLink.print(",");
  espLink.print(pumpOn ? 1 : 0);
  espLink.print(",");
  espLink.println(tankConfigured ? 1 : 0);

  // 8. Serial Monitor
  Serial.print("NEPA: ");
  Serial.print(nepaReading);

  Serial.print(" | LDR: ");
  Serial.print(lightReading);

  Serial.print(" | Source: ");

  if (desiredSource == SOURCE_NEPA)
    Serial.print("NEPA");
  else if (desiredSource == SOURCE_SOLAR)
    Serial.print("SOLAR");
  else
    Serial.print("NONE");

  Serial.print(" | Tank: ");

  if (tankConfigured) {
    Serial.print(tankHeight);
    Serial.print(" cm");
  }
  else {
    Serial.print("NOT CONFIGURED");
  }

  Serial.print(" | Distance: ");
  Serial.print(distance);

  Serial.print(" cm | Water: ");
  Serial.print(waterHeight);

  Serial.print(" cm | Level: ");
  Serial.print(percentage);

  Serial.print("% | Pump: ");
  Serial.println(pumpOn ? "ON" : "OFF");

  delay(250);
}
