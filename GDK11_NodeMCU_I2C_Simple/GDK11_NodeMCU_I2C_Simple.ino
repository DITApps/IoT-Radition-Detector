
#include <Wire.h>

int addr = 0x18;
int __day, hour, _min, _sec = 0;

byte buffer[2] = {0, 0};
int status = 0;

void setup() {
  //Arduino Initialize
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Gamma Sensor Sensing Start");
  //Read Firmware Version
  Gamma_Mod_Read(0xB4);
  //Reset before operating the sensor
  Gamma_Mod_Read(0xA0);
  Serial.println("================================================");
}

void loop() {
  delay(1000);
  Gamma_Mod_Read(0xB0); // Read Status
  Gamma_Mod_Read(0xB1); // Read Measuring Time
  Gamma_Mod_Read(0xB2); // Read Measuring Value (10_min avg.)
  Gamma_Mod_Read(0xB3); // Read Measuring Value (1_min avg.)
  Serial.println("================================================");
  _sec++;
}

void Gamma_Mod_Read(int cmd) {
  /* Begin Write Sequence */
  Wire.beginTransmission(addr);
  Wire.write(cmd);
  Wire.endTransmission();
  /* End Write Sequence */
  delay(10);
  /* Begin Read Sequence */
  Wire.requestFrom(addr, 2);
  byte i = 0;
  while (Wire.available())
  {
    buffer[i] = Wire.read();
    i++;
  }
  /* End Read Sequence */
  /* View Results */
  Print_Result(cmd);
}

void Print_Result(int cmd) {
  float value = 0.0f;
  switch (cmd) {
    case 0xA0:
      Serial.print("Reset Response\t\t\t");
      if (buffer[0] == 1) Serial.println("Reset Success.");
      else Serial.println("Reset Fail(Status - Ready).");
      break;
    case 0xB0:
      Serial.print("Status\t\t\t\t");
      switch (buffer[0]) {
        case 0: Serial.println("Ready"); break;
        case 1: Serial.println("10_min Waiting"); break;
        case 2: Serial.println("Normal"); break;
      }
      status = buffer[0];
      Serial.print("VIB Status\t\t\t");
      switch (buffer[1]) {
        case 0: Serial.println("OFF"); break;
        case 1: Serial.println("ON"); break;
      }
      break;
    case 0xB1:
      if (status > 0) {
        _sec++;
        Cal_Measuring_Time();
      }
      break;
    case 0xB2:
      Serial.print("Measuring Value(10_min avg)\t");
      value = buffer[0] + (float)buffer[1] / 100;
      Serial.print(value); Serial.println(" uSv/hr");
      break;
    case 0xB3:
      Serial.print("Measuring Value(1_min avg)\t");
      value = buffer[0] + (float)buffer[1] / 100;
      Serial.print(value); Serial.println(" uSv/hr");
      break;
    case 0xB4:
      Serial.print("FW Version\t\t\t");
      Serial.print("V"); Serial.print(buffer[0]);
      Serial.print("."); Serial.println(buffer[1]);
      break;
  }
}

/*
  Calculation Measuring Time
  Format :: 0d 00:00:00 ( (__day)d (hour):(_min):(_sec) )
*/
void Cal_Measuring_Time() {
  //int _min;

  if (_sec == 60) {
    _sec = 0;
    _min++;
  }
  if (_min == 60) {
    _min = 0;
    hour++;
  }
  if (hour == 24) {
    hour = 0;
    __day++;
  }
  Serial.print("Measuring Time\t\t\t");
  Serial.print(__day); Serial.print("d ");
  if (hour < 10) Serial.print("0");
  Serial.print(hour); Serial.print(":");
  if (_min < 10) Serial.print("0");
  Serial.print(_min); Serial.print(":");
  if (_sec < 10) Serial.print("0");
  Serial.println(_sec);
}
