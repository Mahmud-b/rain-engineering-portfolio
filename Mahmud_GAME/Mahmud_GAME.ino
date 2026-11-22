
#include <SoftwareSerial.h>

SoftwareSerial espSerial(10, 11);

// Joystick
const int steeringPin = A0;
const int throttlePin = A1;
const int speedPin = A2;
const int joystickButton = 2;

// LEDs and buzzer
const int greenLED = 7;
const int blueLED = 4;
const int buzzer = 8;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(joystickButton, INPUT_PULLUP);

  Serial.println("CAR CONTROLLER STARTED");
}

void loop() {
  int steering = analogRead(steeringPin);
  int throttle = analogRead(throttlePin);
  int maxSpeed = analogRead(speedPin);

  // Send controller values to NodeMCU
  espSerial.print(steering);
  espSerial.print(",");
  espSerial.print(throttle);
  espSerial.print(",");
  espSerial.println(maxSpeed);

  // Forward and reverse indicators
  if (throttle > 562) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(blueLED, LOW);
  }
  else if (throttle < 462) {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH);
  }
  else {
    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, LOW);
  }

  // Joystick button = car horn
 if (digitalRead(joystickButton) == LOW) {
  tone(buzzer, 1000);
}
else {
  noTone(buzzer);
}

  delay(50);
}
