
const int enA = 3;
const int enB = 5;

const int in1 = A0;
const int in2 = 4;
const int in3 = A1;
const int in4 = 6;

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  analogWrite(enA, 100);
  analogWrite(enB, 100);

  // Both motors commanded forward
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void loop() {
}
