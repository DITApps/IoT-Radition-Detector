// Gamma Sensor's Example Interface

// Adafruit OLED 128x64
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//#include <Wire.h>
/*
* We will be using the I2C hardware interface on the Arduino in
* combination with the built-in Wire library to interface.
* Arduino analog input 5 - I2C SCL
* Arduino analog input 4 - I2C SDA
*
* Command List
* 0xA0 :: Reset
* 0xB0 :: Read Status
* 0xB1 :: Read Measuring Time
* 0xB2 :: Read Measuring Value (10min avg / 1min update)
* 0xB3 :: Read Measuring Value (1min avg / 1min update)
* 0xB4 :: Read Firmware Version
*
* Address Assignment
* Default Address :: 0x18
* A0 Open, A1 Short :: 0x19
* A0 Short, A1 Open :: 0x1A
* A0 Open, A1 Open :: 0x1B
*/
int addr = 0x18;

int _day, _hour, _min, _sec = 0;
byte buffer[2] = {0,0};
int status = 0;

void setup() {
  //Arduino Initialize
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Gamma Sensor Sensing Start");
  //Read Firmware version
  Gamma_Mod_Read(0xB4);
  //Reset before operating the sensor
  Gamma_Mod_Read(0xA0);
  Serial.println("================================================");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);// initialize with the I2C addr 0x3C
  display.display();
  display.clearDisplay();
}

void loop() {
  delay(1000);
  //Read Statue, Measuring Time, Measuring Value
  Gamma_Mod_Read_Value();
  Serial.println("================================================");
  delay(500);
}

void Gamma_Mod_Read_Value(){
  Gamma_Mod_Read(0xB0); // Read Status
  Gamma_Mod_Read(0xB1); // Read Measuring Time
  Gamma_Mod_Read(0xB2); // Read Measuring Value (10min avg / 1min update)
  Gamma_Mod_Read(0xB3); // Read Measuring Value (1min avg / 1min update)
}

void Gamma_Mod_Read(int cmd){
  /* Begin Write Sequence */
  Wire.beginTransmission(addr);
  Wire.write(cmd);
  Wire.endTransmission();
  /* End Write Sequence */
  delay(10);
  /* Begin Read Sequence */
  Wire.requestFrom(addr, 2);
  byte i = 0;
  while(Wire.available())
  {
    buffer[i] = Wire.read();
    i++;
  }
  /* End Read Sequence */
  /* View Results */
  Print_Result(cmd);
}
/*
* Calculation Measuring Time
* Format :: 0d 00:00:00 ( (day)d (hour):(min):(sec) )
*/
void Cal_Measuring_Time(){
  if(_sec == 60) { _sec = 0; _min++; }
  if(_min == 60) { _min = 0; _hour++; }
  if(_hour == 24) { _hour = 0; _day++; }
  Serial.print("Measuring Time\t\t\t");
  Serial.print(_day); Serial.print("d ");
  if(_hour < 10) Serial.print("0");
  Serial.print(_hour); Serial.print(":");
  if(_min < 10) Serial.print("0");
  //Serial.print(); 
  Serial.print(":");
  if(_sec < 10) Serial.print("0");
  Serial.println(_sec);
}

void Print_Result(int cmd){
  float value = 0.0f;
  switch(cmd){
    case 0xA0:
    Serial.print("Reset Response\t\t\t");
    if(buffer[0]== 1) Serial.println("Reset Success.");
    else Serial.println("Reset Fail(Status - Ready).");
    break;
    
    case 0xB0:
    Serial.print("Status\t\t\t\t");
    switch(buffer[0]){
      case 0: Serial.println("Ready"); break;
      case 1: Serial.println("10min Waiting"); break;
      case 2: Serial.println("Normal"); break;
    }
    status = buffer[0];
    Serial.print("VIB Status\t\t\t");
    switch(buffer[1]){
      case 0: Serial.println("OFF"); break;
      case 1: Serial.println("ON"); break;
    }
    break;
    
    case 0xB1:
    if(status > 0){
      _sec++;
      Cal_Measuring_Time();
    }
    break;
    
    case 0xB2:
    Serial.print("Measuring Value(10min avg)\t");
    value = buffer[0] + (float)buffer[1]/100;
    Serial.print(value); Serial.println(" uSv/hr");

//  OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 32);
    display.print("10 min avg value");

    display.setCursor(0, 42);
    display.setTextSize(2);
    display.print(value);
    display.print(" uSv/h");
  
    //display.display();
    //delay(500);
    
    break;
    
    case 0xB3:
    Serial.print("Measuring Value(1min avg)\t");
    value = buffer[0] + (float)buffer[1]/100;
    Serial.print(value); Serial.println(" uSv/hr");
    
//  OLED Display
    //display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("1 min avg value");

    display.setCursor(0, 10);
    display.setTextSize(2);
    display.print(value);
    display.print(" uSv/h");
  
    display.display();
    delay(500);
    break;
    
    case 0xB4:
    Serial.print("FW Version\t\t\t");
    Serial.print("V"); Serial.print(buffer[0]);
    Serial.print("."); Serial.println(buffer[1]);
    break;
  }
}
