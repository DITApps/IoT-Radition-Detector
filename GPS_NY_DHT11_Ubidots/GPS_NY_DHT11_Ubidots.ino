// GPS GY-NE, HDT11, Ubidots
#include <DHT.h>
#include "UbidotsMicroESP8266.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Ubidots Business : IoTBusan
#define TOKEN  "BBFF-H33IAugaKzWOs1sS1thrIzCP0nQNu7"  // Put here your Ubidots TOKEN

//#define WIFISSID "melon" // Put here your Wi-Fi SSID
//#define PASSWORD "deitcs3217" // Put here your Wi-Fi password
//#define WIFISSID "Amadeus" // Put here your Wi-Fi SSID
//#define PASSWORD "deitcs3217" // Put here your Wi-Fi password
#define WIFISSID "olleh_WiFi_0F8E" // Put here your Wi-Fi SSID
#define PASSWORD "0000006593" // Put here your Wi-Fi password

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
  
  client.wifiConnection(WIFISSID, PASSWORD);
  Serial.print("Wi-Fi Conneted!!");
  //client.setDataSourceName("gps-tracker");
}

void loop()
{ 
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
          
          Serial.print(F("Location: ")); 
          Serial.print(_lat);
          Serial.print(F(","));
          Serial.print(_lng);
          Serial.print(F("Humi: "));
          Serial.print(_humi);
          Serial.print(F("Temp: "));
          Serial.println(_temp);
          
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
