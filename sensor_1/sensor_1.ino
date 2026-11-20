
int Trigpin = 12;
int echopin = 11;

void setup() {
  Serial.begin(9600);
  pinMode(Trigpin, OUTPUT);
  pinMode(echopin, INPUT);
}

void loop() {
  digitalWrite(Trigpin, LOW);
  delayMicroseconds(2);

  digitalWrite(Trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigpin, LOW);

  long duration = pulseIn(echopin, HIGH, 30000);

  if (duration == 0) {
    Serial.println("No echo detected");
  } else {
    float distance = 0.01715 * duration;
    float x = (10.6 - distance);
    float y = (x/10.6);
    float percentage = y * 100;
  
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm");
    Serial.print(" | Percentage: ");
    Serial.print(percentage);
    Serial.println("%");
  }

  delay(500);
}
