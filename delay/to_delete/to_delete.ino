int x = 1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (x<=6){
  Serial.println(x);
  x+=1;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
