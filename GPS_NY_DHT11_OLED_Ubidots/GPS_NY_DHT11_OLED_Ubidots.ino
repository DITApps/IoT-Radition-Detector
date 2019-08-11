// GPS GY-NE, HDT11, SSD1306, Ubidots
// SDA : D1, SCL: D2
#include <DHT.h>
#include "UbidotsMicroESP8266.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  display(0x3c, 5, 4); // Initialise the OLED display using Wire library

// Ubidots Business : IoTBusan
#define TOKEN  "BBFF-H33IAugaKzWOs1sS1thrIzCP0nQNu7"  // Put here your Ubidots TOKEN

//#define WIFISSID "melon" // Put here your Wi-Fi SSID
//#define PASSWORD "deitcs3217" // Put here your Wi-Fi password
#define WIFISSID "Amadeus" // Put here your Wi-Fi SSID
#define PASSWORD "deitcs3217" // Put here your Wi-Fi password
//#define WIFISSID "olleh_WiFi_0F8E" // Put here your Wi-Fi SSID
//#define PASSWORD "0000006593" // Put here your Wi-Fi password

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Ubidots client(TOKEN);

static const int RXPin = D7, TXPin = D6;
static const uint32_t GPSBaud = 9600; // Change according to your device

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

double _lat, _lng;
char context[25];
float _temp;
float _humi;

unsigned long previousMillis = 0;     // last time data was send
const long interval = 1000;           // data transfer interval 

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  dht.begin();
  display.init(); // Initialising the UI will init the display too.
  display.flipScreenVertically();
  display.clear();
  //display.display();
  
  client.wifiConnection(WIFISSID, PASSWORD);
  Serial.print("Wi-Fi Conneted!!");
  //client.setDataSourceName("gps-tracker");
}

void loop()
{ 
  int x=0;
  int y=0;
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      unsigned long currentMillis = millis();
  
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        if (gps.location.isValid())
        {
          _lat = gps.location.lat();
          _lng = gps.location.lng();
          sprintf(context, "lat=%.3f$lng=%.3f", _lat, _lng);

          _temp = dht.readTemperature();
          _humi = dht.readHumidity();

          client.add("humi", _humi, context);
          client.add("temp", _temp, context);
          //client.add("location ", _temp, context);
          client.sendAll(true);

          display.clear();
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0 + x, 5 + y, "Hum");
          
//          display.setFont(ArialMT_Plain_10);
//          display.setTextAlignment(TEXT_ALIGN_LEFT);
//          display.drawString(43 + x, y, "GPS Tracker");
        
          display.setFont(ArialMT_Plain_24);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          String hum = String(int(_humi)) + "%";
          display.drawString(0 + x, 15 + y, hum);
          int humWidth = display.getStringWidth(hum);
        
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(95 + x, 5 + y, "Temp");
        
          display.setFont(ArialMT_Plain_24);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          String temp = String(int(_temp)) + "°C";
          display.drawString(70 + x, 15 + y, temp);
          int tempWidth = display.getStringWidth(temp);

          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0 + x, 45 + y, "Lat : ");

          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(23 + x, 45 + y, String(_lat));

          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(66 + x, 45 + y, "Long : ");
          
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(97 + x, 45 + y, String(_lng));
          
          display.display();        
//           
//          client.add("humi", _humi, context);
//          client.add("temp", _temp, context);
//          //client.add("location ", _temp, context);
//          client.sendAll(true);
          
          Serial.print(F("Location: ")); 
          Serial.print(_lat);
          Serial.print(F(","));
          Serial.print(_lng);
          Serial.print(F("  Humi: "));
          Serial.print(_humi);
          Serial.print(F("  Temp: "));
          Serial.print(_temp);
          
        }
        else
        {
          Serial.print(F("INVALID"));
        }
        
        Serial.println();
      }
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}
