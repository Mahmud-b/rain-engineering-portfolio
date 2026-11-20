int statuspin = 7;
int the_status;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(statuspin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  the_status = digitalRead(statuspin);
  Serial.print("status: ");
  Serial.println(the_status);
}
