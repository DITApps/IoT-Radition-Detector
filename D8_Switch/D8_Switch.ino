const int buttonPin = 15; // GPIO 15 - D8
const int ledPin = 2;    // GPIO 2 - D4 - builtin led

int buttonState = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  // pinMode(buttonPin, INPUT_PULLUP); // pullup 지정을 해도 pulldown으로 동작
  pinMode(buttonPin, INPUT);
}

void LedOn() {
    digitalWrite(ledPin, LOW);
}

void LedOff() {
    digitalWrite(ledPin, HIGH);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) { // 스위치가 닫히면,
    LedOn();
  } else { // 스위치가 열리면,             
    LedOff();
  }

  delay(1000);
}
