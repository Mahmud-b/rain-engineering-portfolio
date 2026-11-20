
#include <SoftwareSerial.h>

SoftwareSerial esp(7, 8); // RX, TX

// L298N pins
const int enA = 3;
const int enB = 5;

const int in1 = A0;
const int in2 = 4;
const int in3 = A1;
const int in4 = 6;

const int motorSpeedA = 180;
const int motorSpeedB = 180;

// Safety timeout
unsigned long lastCommandTime = 0;
const unsigned long timeout = 700;

void stopMotor() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);
}

void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);
}

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.begin(9600);
  esp.begin(9600);

  stopMotor();

  Serial.println("Wireless robot ready!");
}

void loop() {
  if (esp.available()) {
    String command = esp.readStringUntil('\n');
    command.trim();

    lastCommandTime = millis();

    if (command == "F") {
      forward();
    }
    else if (command == "B") {
      backward();
    }
    else if (command == "L") {
      turnLeft();
    }
    else if (command == "R") {
      turnRight();
    }
    else if (command == "S") {
      stopMotor();
    }
  }

  // Stop if commands stop arriving
  if (millis() - lastCommandTime > timeout) {
    stopMotor();
  }
}
