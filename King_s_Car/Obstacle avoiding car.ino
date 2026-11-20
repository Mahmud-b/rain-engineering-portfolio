#include <Servo.h>

const int enA = 3;
const int enB = 5;

const int in1 = A0;
const int in2 = 4;
const int in3 = A1;
const int in4 = 6;

const int motorSpeedA = 180;
const int motorSpeedB = 180;

const int trigPin = 12;
const int echoPin = 2;

Servo scanner;
const int servoPin = 11;

const int safeDistance = 25;

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) {
    return -1;
  }

  int distance = duration * 0.0343 / 2;

  return distance;
}

void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void stopMotor() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);

  scanner.attach(servoPin);

  Serial.begin(9600);

  scanner.write(90);
  delay(500);

  stopMotor();
}

void loop() {
  scanner.write(90);
  delay(200);

  int frontDistance = getDistance();

  Serial.print("Front: ");
  Serial.println(frontDistance);

  if (frontDistance == -1) {
    Serial.println("Front sensor error!");

    stopMotor();
    delay(200);
  }
  else if (frontDistance > safeDistance) {
    forward();
  }
  else {
    Serial.println("Obstacle detected!");

    stopMotor();
    delay(300);

    backward();
    delay(300);

    stopMotor();
    delay(200);

    scanner.write(150);
    delay(500);

    int leftDistance = getDistance();

    scanner.write(30);
    delay(500);

    int rightDistance = getDistance();

    scanner.write(90);
    delay(300);

    Serial.print("Left: ");
    Serial.println(leftDistance);

    Serial.print("Right: ");
    Serial.println(rightDistance);

    int leftClearance = leftDistance == -1 ? 0 : leftDistance;

    int rightClearance = rightDistance == -1 ? 0 : rightDistance;

    if (leftClearance > safeDistance &&
        leftClearance > rightClearance) {
      Serial.println("Turning LEFT");

      turnLeft();
      delay(450);
    }
    else if (rightClearance > safeDistance) {
      Serial.println("Turning RIGHT");

      turnRight();
      delay(450);
    }
    else {
      Serial.println("Both paths blocked!");

      stopMotor();
    }

    stopMotor();
    delay(200);
  }
}
