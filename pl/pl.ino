void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 int a = 1;
 int b = 6;
 int c = 9;
 String password = "196";
 String d = String(a);
 String e = String(b);
 String f = String(c);
 String x = String(d+e+f);
  if (x == "169"){
    Serial.print("You're Welcome");
    }

}

void loop() {
  // put your main code here, to run repeatedly:

}
