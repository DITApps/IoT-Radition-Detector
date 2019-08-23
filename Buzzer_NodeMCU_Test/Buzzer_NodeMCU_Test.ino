static const int BuzzerPin = D5;

void setup() {
  // put your setup code here, to run once:
   // Buzzer init
  pinMode(BuzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Buzzer test
    tone(BuzzerPin, 500);
    delay(100);
    noTone(BuzzerPin);
    delay(100);

}
