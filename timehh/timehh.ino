#include <ESP8266WiFi.h>
 #include <time.h>
const char* ssid = "201915765";           // your ssid
const char* password = "ghg912310";      // your password
int timezone = 3;
int dst = 0;

void setup() {
Serial.begin(115200);
Serial.setDebugOutput(true);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
Serial.println("\nConnecting to WiFi");
while (WiFi.status() != WL_CONNECTED) {
Serial.print(".");
delay(1000);
}


 
configTime(9 * 3600+40, 0, "pool.ntp.org", "time.nist.gov");
Serial.println("\nWaiting for time");
while (!time(nullptr)) {
Serial.print(".");
delay(1000);
}
Serial.println("");
}

String getTextTime(time_t now)
{
struct tm * timeinfo;
timeinfo = localtime(&now);
// Serial.println(timeinfo->tm_hour);
// Serial.println(timeinfo->tm_min);
// Serial.println(timeinfo->tm_wday);
String text = String(timeinfo->tm_hour)+String(timeinfo->tm_min)+String(timeinfo->tm_wday);
//Serial.println(text);
}


 
void loop() {
time_t now = time(nullptr);
Serial.println(ctime(&now));
getTextTime(now);
delay(1000);
}
