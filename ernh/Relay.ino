
#define LDR_PIN A0
#define NEPA_INPUT 2

#define BLUE_RELAY 9
#define BLACK_RELAY 7

#define SOLAR_THRESHOLD 500

// Both relay modules are assumed active-LOW.
#define RELAY_ON LOW
#define RELAY_OFF HIGH

enum Source {
  SOURCE_OFF,
  SOURCE_NEPA,
  SOURCE_SOLAR
};

Source currentSource = SOURCE_OFF;

void setup() {
  Serial.begin(9600);

  pinMode(NEPA_INPUT, INPUT_PULLUP);

  pinMode(BLUE_RELAY, OUTPUT);
  pinMode(BLACK_RELAY, OUTPUT);

  // Start with both relays OFF.
  digitalWrite(BLACK_RELAY, RELAY_OFF);
  digitalWrite(BLUE_RELAY, RELAY_OFF);

  Serial.println("SMARTTANK SOURCE CONTROLLER");
}

void loop() {
  int lightValue = analogRead(LDR_PIN);

  bool nepaAvailable =
    (digitalRead(NEPA_INPUT) == LOW);

  bool solarAvailable =
    (lightValue >= SOLAR_THRESHOLD);

  Source desiredSource;

  // NEPA has priority.
  if (nepaAvailable) {
    desiredSource = SOURCE_NEPA;
  }
  else if (solarAvailable) {
    desiredSource = SOURCE_SOLAR;
  }
  else {
    desiredSource = SOURCE_OFF;
  }

  // Change source only when necessary.
  if (desiredSource != currentSource) {

    // Disconnect output before changing source.
    digitalWrite(BLACK_RELAY, RELAY_OFF);
    delay(500);

    if (desiredSource == SOURCE_NEPA) {
      // Blue relay OFF selects its NC terminal.
      digitalWrite(BLUE_RELAY, RELAY_OFF);
    }

    else if (desiredSource == SOURCE_SOLAR) {
      // Blue relay ON selects its NO terminal.
      digitalWrite(BLUE_RELAY, RELAY_ON);
    }

    delay(500);

    if (desiredSource != SOURCE_OFF) {
      digitalWrite(BLACK_RELAY, RELAY_ON);
    }

    currentSource = desiredSource;
  }

  Serial.print("Light: ");
  Serial.print(lightValue);

  Serial.print(" | NEPA: ");
  Serial.print(nepaAvailable ? "AVAILABLE" : "UNAVAILABLE");

  Serial.print(" | SOURCE: ");

  if (currentSource == SOURCE_NEPA) {
    Serial.println("NEPA");
  }
  else if (currentSource == SOURCE_SOLAR) {
    Serial.println("SOLAR");
  }
  else {
    Serial.println("OFF");
  }

  delay(500);
}
