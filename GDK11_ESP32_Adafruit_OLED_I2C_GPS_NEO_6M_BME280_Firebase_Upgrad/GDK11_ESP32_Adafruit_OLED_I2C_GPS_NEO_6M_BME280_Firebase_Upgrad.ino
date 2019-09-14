///////////////////////////////////////
// Firebase, GPS, OLED, 감마센서, BME280
////////////////////////////////////////
// Gamma Sensor's Example Interface
// GPS 최기화 전에도 감마 센서 작동
//#include "UbidotsMicroESP8266.h"

// Firebase
#include <time.h>
#include <ESP8266WiFi.h>                                                    // esp8266 library
#include <FirebaseArduino.h>    // firebase library                                                      // dht11 temperature and humidity sensor library
#define FIREBASE_HOST "radiationtracker-1a73e.firebaseio.com"                          // the project name address from firebase id
#define FIREBASE_AUTH "bs86QcuVBSFH4qBEMTf3WWEz0i8WRJYGjYPcNZSL"            // the secret key generated from firebase

// BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

#define WIFI_SSID "melon"                                             // input your home or public wifi name 
#define WIFI_PASSWORD "deitcs3217"   
//#define WIFI_SSID "Amadeus"                                             // input your home or public wifi name 
//#define WIFI_PASSWORD "deitcs3217" 
//#define WIFI_SSID "olleh_WiFi_0F8E" // Put here your Wi-Fi SSID
//#define WIFI_PASSWORD "0000006593" // Put here your Wi-Fi password

// Adafruit OLED 128x64
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// GPS 라이브러리
#include <TinyGPS++.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Ubidots Business : .IoTBusan
//#define TOKEN  "BBFF-H33IAugaKzWOs1sS1thrIzCP0nQNu7"  // Put here your Ubidots TOKEN

// OLED 최기화 : Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// buzzer Siren 설정 
int freq = 150;
boolean freqFlag = true;

#include <SoftwareSerial.h>

/*
  We will be using the I2C hardware interface on the Arduino in
  combination with the built-in Wire library to interface.
  Arduino analog input 5 - I2C SCL
  Arduino analog input 4 - I2C SDA

  Command List
  0xA0 :: Reset
  0xB0 :: Read Status
  0xB1 :: Read Measuring Time
  0xB2 :: Read Measuring Value (10min avg / 1min update)
  0xB3 :: Read Measuring Value (1min avg / 1min update)
  0xB4 :: Read Firmware Version

  Address Assignment
  Default Address :: 0x18
  A0 Open, A1 Short :: 0x19
  A0 Short, A1 Open :: 0x1A
  A0 Open, A1 Open :: 0x1B
*/
int addr = 0x18;
int _day, _hour, _min, _sec = 0;
byte buffer[2] = {0, 0};
int status = 0;

// 시간 설정
int timezone = 3;
int dst = 0;

// GPS 설정
static const int RXPin = D7, TXPin = D6; // GPS Software Serial
static const int buzzerPin = D5; // 부저 PWM
static const uint32_t GPSBaud = 9600; // Change according to your device

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// 위도, 경도 저장
float _lat, _lng;

// 감마센서 보정값 저장
float val_1min = 0.0f;
float val_10min = 0.0f;
int val_alert = 0;

// BME208 altitude Value
float _altitude = 0.00f;

void setup() {
  //Arduino Initialize
  Serial.begin(115200);
  Wire.begin();
  // Buzzer init
  pinMode(buzzerPin, OUTPUT);
  // GPS init
  ss.begin(GPSBaud);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);// initialize with the I2C addr 0x3C
  display.display();
  display.clearDisplay();

  // BME I2C setup
  bool status;
  status = bme.begin(0x76);  // 0x76 
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }
  Serial.println("BME OK");

  Serial.println("Gamma Sensor Sensing Start");
  //Read Firmware version
  Gamma_Mod_Read(0xB4);
  //Reset before operating the sensor
  Gamma_Mod_Read(0xA0);

  // Firebase Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 

  // 시간 초기화
  configTime(9 * 3600 + 40, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop()
{ 
  // 시간 출력
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  getTextTime(now);
  
  // Buzzer Alerm Start
  val_alert = val_1min * 100;
  Serial.println(val_alert);
  switch (val_alert) {
    case 1 ... 39 :  // 방사선 발견시
      buzzerAlert(0, 500, 500);
    case 40 ... 299 : // 경고 수준
      buzzerAlert(2000, 500, 100);
      break;
    case 300 ... 5000 :  // 위험 수준
      //buzzerAlert(65535, 0, 0);
      buzzerSiren();
      break;
  }
   
  //Read Statue, Measuring Time, Measuring Value
  Gamma_Mod_Read_Value();
  Serial.println("================================================");

  // BME print
  _altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("Approx. Altitude = ");
  Serial.print(_altitude);
  Serial.println(" m");
    
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject&root = jsonBuffer.createObject();
  root["nowTime"] = ctime(&now);
  root["value1min"] = val_1min;
  root["value10min"] = val_10min;
  root["latitude"] = _lat;
  root["longitude"] = _lng;
  root["altitude"] = _altitude;
  Firebase.push("/data", root);
  delay(10000);
}

void Gamma_Mod_Read_Value() {
  Gamma_Mod_Read(0xB0); // Read Status
  Gamma_Mod_Read(0xB1); // Read Measuring Time
  Gamma_Mod_Read(0xB2); // Read Measuring Value (10min avg / 1min update)
  Gamma_Mod_Read(0xB3); // Read Measuring Value (1min avg / 1min update)
}

void Gamma_Mod_Read(int cmd)
{
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
/*
  Calculation Measuring Time
  Format :: 0d 00:00:00 ( (day)d (hour):(min):(sec) )
*/
void Cal_Measuring_Time() {
  if (_sec == 60) {
    _sec = 0;
    _min++;
  }
  if (_min == 60) {
    _min = 0;
    _hour++;
  }
  if (_hour == 24) {
    _hour = 0;
    _day++;
  }
  Serial.print("Measuring Time\t\t\t");
  Serial.print(_day); Serial.print("d ");
  if (_hour < 10) Serial.print("0");
  Serial.print(_hour); Serial.print(":");
  if (_min < 10) Serial.print("0");
  //Serial.print();
  Serial.print(":");
  if (_sec < 10) Serial.print("0");
  Serial.println(_sec);
}

void Print_Result(int cmd) {
  float value = 0.0f;
  /////
  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        _lat = gps.location.lat();
        _lng = gps.location.lng();
        //sprintf(context, "lat=%.3f$lng=%.3f", _lat, _lng);
        //Serial.print(context);
        Serial.print("GPS lat  ");
        Serial.print(_lat);

      } else {
        Serial.print(F("GPS INVALID"));
      }
      Serial.println();
    }
  }

  //delay(1000);

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
        case 1: Serial.println("10min Waiting"); break;
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
      Serial.print("Measuring Value(10min avg)\t");
      value = buffer[0] + (float)buffer[1] / 100;
      val_10min = value / 12.30;
      Serial.print(val_10min); Serial.println(" uSv/hr");

      //  OLED Display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 32);
      display.print("10 min avg value");

      display.setCursor(0, 42);
      display.setTextSize(2);
      display.print(val_10min);
      display.setCursor(50, 49);
      display.setTextSize(1);
      display.print(" uSv/hr");

      //display.display();
      //delay(500);

      break;

    case 0xB3:
      Serial.print("Measuring Value(1min avg)\t");
      value = buffer[0] + (float)buffer[1] / 100;
      val_1min = value / 12.30;
      Serial.print(val_1min); Serial.println(" uSv/hr");
      
      //  OLED Display
      //display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("1 min avg value");

      display.setCursor(0, 10);
      display.setTextSize(2);
      display.print(val_1min);
      display.setCursor(50, 17);
      display.setTextSize(1);
      display.print(" uSv/hr");

      display.display();
      //delay(500);
      break;

    case 0xB4:
      Serial.print("FW Version\t\t\t");
      Serial.print("V"); Serial.print(buffer[0]);
      Serial.print("."); Serial.println(buffer[1]);
      break;
  }
}

// 부저 함
void buzzerAlert(int freq, int duration, int _delay) {
     delay(_delay); 
     tone(buzzerPin, freq, duration);
     delay(_delay);
     noTone(buzzerPin);
     delay(_delay);  
}

void buzzerSiren() {
    tone(buzzerPin, freq, 10);
    if(freqFlag == true) freq += 2;
    if(freq >= 1800) freqFlag = false;
  
    if(freqFlag == false) freq -= 2;
    if(freq <= 150) freqFlag = true;
    delay(5); 
}

// 시간 구하기 함수
String getTextTime(time_t now)
{
  struct tm * timeinfo;
  timeinfo = localtime(&now);
  // Serial.println(timeinfo->tm_hour);
  // Serial.println(timeinfo->tm_min);
  // Serial.println(timeinfo->tm_wday);
  String text = String(timeinfo->tm_hour) + String(timeinfo->tm_min) + String(timeinfo->tm_wday);
  //Serial.println(text);
}
