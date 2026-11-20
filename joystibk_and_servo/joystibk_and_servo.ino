#include <Servo.h>

Servo myServo1;
Servo myServo2;

int xPin = A0;
int yPin = A1;

int servoPin1 = 9;
int servoPin2 = 10;

int xValue = 0;
int yValue = 0;

int anglex = 0;
int angley = 0;

void setup() {
  Serial.begin(9600);

  myServo1.attach(servoPin1);
  myServo2.attach(servoPin2);
}

void loop() {
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);

  anglex = map(xValue, 0, 1023, 0, 180);
  angley = map(yValue, 0, 1023, 0, 180);

  myServo1.write(anglex);
  myServo2.write(angley);
  Serial.print("X value: "); 
  Serial.print(xValue);
  Serial.print("Y value: ");   
  Serial.println(yValue); 
  delay(20);
}
