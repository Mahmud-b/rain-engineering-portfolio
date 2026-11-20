
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int xpin = A0;
int ypin = A1;
int buttonPin = 2;

int key1 = 0;
int key2 = 0;

String password = "UDRL";
String x = "";

bool Uppressed = false;
bool Downpressed = false;
bool Rightpressed = false;
bool Leftpressed = false;

bool correctpassword = false;

int lastButtonState = HIGH;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.setCursor(0, 0);
  lcd.print("Enter Password");
}

void resetinput() {
  x = "";

  Uppressed = false;
  Downpressed = false;
  Rightpressed = false;
  Leftpressed = false;

  correctpassword = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password");
}

void recordInput(char direction) {
  if (x.length() < password.length()) {
    x += direction;

    lcd.setCursor(0, 0);
    lcd.print("Code: ");
    lcd.print(x);
    lcd.print("       ");
  }
}

void input() {
  if (correctpassword) {
    return;
  }

  key1 = analogRead(xpin);
  key2 = analogRead(ypin);

  // UP
  if (key1 > 900 && !Uppressed) {
    recordInput('U');
    Uppressed = true;
  }

  if (key1 < 800) {
    Uppressed = false;
  }

  // DOWN
  if (key1 < 100 && !Downpressed) {
    recordInput('D');
    Downpressed = true;
  }

  if (key1 > 200) {
    Downpressed = false;
  }

  // RIGHT
  if (key2 > 900 && !Rightpressed) {
    recordInput('R');
    Rightpressed = true;
  }

  if (key2 < 800) {
    Rightpressed = false;
  }

  // LEFT
  if (key2 < 100 && !Leftpressed) {
    recordInput('L');
    Leftpressed = true;
  }

  if (key2 > 200) {
    Leftpressed = false;
  }
}

void loop() {

  // Reset button: detect a new press
  int buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(30);

    if (digitalRead(buttonPin) == LOW) {
      resetinput();
      lastButtonState = LOW;
      return;
    }
  }

  lastButtonState = buttonState;

  input();

  // Check password after four directions
  if (x.length() == password.length() && !correctpassword) {

    if (x == password) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Master :)");

      lcd.setCursor(0, 1);
      lcd.print("Access Granted");

      correctpassword = true;
    }

    else {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wrong Password");

      lcd.setCursor(0, 1);
      lcd.print("Try Again");

      delay(2000);

      resetinput();
    }
  }
}
 
