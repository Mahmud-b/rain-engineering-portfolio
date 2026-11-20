int potentiometer = A0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(potentiometer);
  float y = map(value,0,1023,0,100);
  Serial.println(y);
}
