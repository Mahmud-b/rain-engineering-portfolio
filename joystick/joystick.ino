
int xPin = A0;
int yPin = A1;
int switchPin = 2;

void setup() {
  Serial.begin(9600);

  pinMode(switchPin, INPUT_PULLUP);
}

void loop() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  int buttonValue = digitalRead(switchPin);

  Serial.print("X: ");
  Serial.print(xValue);

  Serial.print(" | Y: ");
  Serial.print(yValue);

  Serial.print(" | Button: ");
  Serial.println(buttonValue);

  delay(200);
}
