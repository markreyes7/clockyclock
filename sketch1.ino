/**
 * RTC_NTPSync
 * 
 * This example shows how to set the RTC (Real Time Clock) on the Portenta C33 / UNO R4 WiFi
 * to the current date and time retrieved from an NTP server on the Internet (pool.ntp.org).
 * Then the current time from the RTC is printed to the Serial port.
 * 
 * Instructions:
 * 1. Download the NTPClient library (https://github.com/arduino-libraries/NTPClient) through the Library Manager
 * 2. Change the WiFi credentials in the arduino_secrets.h file to match your WiFi network.
 * 3. Upload this sketch to Portenta C33 / UNO R4 WiFi.
 * 4. Open the Serial Monitor.
 * 
 * Initial author: Sebastian Romero @sebromero
 * 
 * Find the full UNO R4 WiFi RTC documentation here:
 * https://docs.arduino.cc/tutorials/uno-r4-wifi/rtc
 */

// Include the RTC library
#include "RTC.h"
//Include the NTP library
#include <NTPClient.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif
#include <WiFiUdp.h>
#include "secrets.h"
#include "ledmatrix.h" 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char ssid2[]= SECRET_SSID2;
char pass2[] = SECRET_PASS2;
 int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);
unsigned long lastSync = 0;
const unsigned long syncInterval = 600000; // 10 minutes in millis


ArduinoLEDMatrix matrix;

void printWifiStatus() {
  
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void executeResync(){
  if (millis() - lastSync > syncInterval) {
    syncRTCFromNTP();
    lastSync = millis();
    Serial.println("Time has been resynced");
  }
}

void syncRTCFromNTP(){
  auto timeZoneOffsetHours = -7;
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime);
}

void connectToWiFi(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);

    Serial.println("Attempting to connect to second SSID:   ");
    Serial.println(ssid);

    wifiStatus = WiFi.begin(ssid2, pass2);
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();
}

void setup(){
  Serial.begin(115200);
  while (!Serial);

  connectToWiFi();
  RTC.begin();
  Serial.println("\nStarting connection to server...");
  timeClient.begin();
  timeClient.update();

  auto timeZoneOffsetHours = -7;
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  Serial.println("The RTC was just set to: " + String(currentTime));


  Serial.println("Initializing LED Matrix");
  Serial.begin(115200);

  matrix.begin();


  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);

  // add some static text

  const char text[] = "Connected!";

  matrix.textFont(Font_4x6);

  matrix.beginText(0, 1, 0xFFFFFF);

  matrix.println(text);

  matrix.endText();


  matrix.endDraw();


  delay(2000);

}




void loop(){
  RTCTime currentTime;


  // Get current time from RTC

  RTC.getTime(currentTime);


  // Print out date (DD/MM//YYYY)

  Serial.print(currentTime.getDayOfMonth());

  Serial.print("/");

  Serial.print(Month2int(currentTime.getMonth()));

  Serial.print("/");

  Serial.print(currentTime.getYear());

  Serial.print(" - ");


  // Print time (HH/MM/SS)

  Serial.print(currentTime.getHour());

  Serial.print(":");

  Serial.print(currentTime.getMinutes());

  Serial.print(":");

  Serial.println(currentTime.getSeconds());


  //delay(1000);
  // Make it scroll!

  matrix.beginDraw();


  matrix.stroke(0xFF00FF00);

  matrix.textScrollSpeed(180);

  // add the textchar timeStr[6];
  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", currentTime.getHour(), currentTime.getMinutes());
  matrix.beginText(0, 1, 0xFF00FF00);

  matrix.print(+ timeStr);
  matrix.endText(SCROLL_LEFT);
  
  executeResync();

 

  // const uint32_t* frame = getArtFrame("heart");
  // if (frame) matrix.loadFrame(frame);
  // delay(500);

  // const uint32_t* frame2 = getArtFrame("happy");
  // if (frame2) matrix.loadFrame(frame2);
  // delay(500);
  // const uint32_t* frame3 = getArtFrame("one");
  // if (frame3) matrix.loadFrame(frame3);
  // delay(1500);
  // const uint32_t* frame4 = getArtFrame("testnum");
  // if (frame4) matrix.loadFrame(frame4);
  // delay(2500);
}
