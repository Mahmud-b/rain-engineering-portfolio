int led = 13;
int temperature = 40;

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    if (Serial.avalaible() >= 30) {
        digitalWrite(led, HIGH);
        Serial.println("LED ON");
    }
    else if (temperature >= 20) {
        digitalWrite(led, LOW);
        Serial.println("Normal temperature");
    }
    else {
        digitalWrite(led, LOW);
        Serial.println("Cold temperature");
    }

    delay(1000);
}
