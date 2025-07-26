#include "RTC.h"
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



/// different SSID's for my personal networks
char ssid[] = SECRET_SSID;        
char pass[] = SECRET_PASS;   
char ssid2[]= SECRET_SSID2;
char pass2[] = SECRET_PASS2;

bool isTimerSet = false;  // mapped to a button that will set to true when clicked and call display timer. 

int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);
WiFiServer server(80); 

ArduinoLEDMatrix matrix;

unsigned long previousMillis = 0;
const unsigned long interval = 1000;

int countdownSeconds = 30 * 60;  // 30 minutes

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

void connectToWiFi(){
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(15000);

    Serial.println("Attempting to connect to second SSID:   ");
    Serial.println(ssid2);

    wifiStatus = WiFi.begin(ssid2, pass2);
    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();
}

void timerLoop() {
  updateCountdown();

  if (countdownSeconds > 0) {
    displayTimer();
  } else {
    isTimerSet = false;  // stop timer when done
  }
}
void serverSetup(){
  
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/B\">this</a> to set the timer.</p>");
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/O\">this</a> to turn off the timer.</p>");
            client.println();

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        
        if (currentLine.endsWith("GET /B")){
          isTimerSet = true;
          previousMillis = millis();  // reset timer
          countdownSeconds = 30 * 60;
          updateCountdown();
          displayTimer();
        }
        if (currentLine.endsWith("GET /O")){
          isTimerSet = false;
        }
      }
    }

    // Close connection
    client.stop();
    Serial.println("Client disconnected");
  }
}



void updateCountdown() {

  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;

  if (countdownSeconds > 0 && elapsed >= interval) {
    Serial.println("THE FIRST SECONDS");
    int secondsPassed = elapsed / interval;
    Serial.println(secondsPassed);
    countdownSeconds -= secondsPassed;
    previousMillis += secondsPassed * interval;

    int min = countdownSeconds / 60;
    int sec = countdownSeconds % 60;

    Serial.print("Time left: ");
    Serial.print(min);
    Serial.print(":");
    if (sec < 10) Serial.print("0");
    Serial.println(sec);
  }
}



void displayTimer() {
  char timeStr[10];
  int min = countdownSeconds / 60;
  int sec = countdownSeconds % 60;
  sprintf(timeStr, "%02d:%02d", min, sec);

  matrix.beginDraw();
  matrix.stroke(0xFF00FF00);
  matrix.textScrollSpeed(120);
  matrix.beginText(0, 1, 0xFF00FF00); // x, y, color
  matrix.print(+ timeStr);
  matrix.endText(SCROLL_LEFT);
}

// void displayTimer(int timeToSet){}. get timeToSetFromUi BUTTON or something

// 


void setup(){
  Serial.begin(115200);
  while (!Serial);

  connectToWiFi();
  RTC.begin();
  Serial.println("\nStarting connection to server...");
  timeClient.begin();
  timeClient.update();

  server.begin(); //setup server for the timer

  // Get the current date and time from an NTP server and convert
  // it to UTC +2 by passing the time zone offset in hours.
  // You may change the time zone offset to your local one.
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

  // will only show "UNO" (not enough space on the display)

  const char text[] = "UNO r4";

  matrix.textFont(Font_4x6);

  matrix.beginText(0, 1, 0xFFFFFF);

  matrix.println(text);

  matrix.endText();


  matrix.endDraw();


  delay(2000);

}


void loop(){
  serverSetup();  // Always check for incoming requests


  //this will be fixed to check if timer mode has been activated. if so, cancel the connect and then do the timer. if not just do the timer.
  RTCTime currentTime;

  if (isTimerSet == true){
    timerLoop();  // Run countdown + display timer
  }
  else {
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

  matrix.beginDraw();
  matrix.stroke(0xFF00FF00);
  matrix.textScrollSpeed(180);

  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", currentTime.getHour(), currentTime.getMinutes());

  matrix.beginText(0, 1, 0xFF00FF00);
  matrix.print(+ timeStr);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}


 

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

