int x = 1;
int y = map(x,0,5,0,1023);
void setup() {
  // put your setup code here, to run once:
pinMode(5,OUTPUT);
Serial.begin(9600);
Serial.println(y);
analogWrite(5,y);

}

void loop() {
  // put your main code here, to run repeatedly:

}
