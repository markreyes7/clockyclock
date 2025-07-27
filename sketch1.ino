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

bool isTimerSet = false;
int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp;
NTPClient timeClient(Udp);
WiFiServer server(80); 

ArduinoLEDMatrix matrix;

unsigned long previousMillis = 0;
const unsigned long interval = 1000;

int countdownSeconds = 30 * 60;  // 30 minutes

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
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
    delay(15000);
    Serial.println("Attempting to connect to second SSID:   ");
    Serial.println(ssid2);
    wifiStatus = WiFi.begin(ssid2, pass2);
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
    isTimerSet = false;
  }
}

void timerLogic(String request){
  if (request.indexOf("GET /set-timer?minutes=") >= 0) {
    int startIdx = request.indexOf("minutes=") + 8;
    int endIdx = request.indexOf(" ", startIdx);
    String numStr = request.substring(startIdx, endIdx);
    int duration = numStr.toInt();
    if (duration > 0) {
      countdownSeconds = duration * 60;
      previousMillis = millis();
      isTimerSet = true;
      Serial.print("Timer set for minutes: ");
      Serial.println(duration);
    }
  }
}

void serverSetup(){
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        Serial.write(c);

        if (c == '\n') {
          if (currentLine.length() == 0) {
            timerLogic(request);
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/B\">this</a> to set the timer.</p>");
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/O\">this</a> to turn off the timer.</p>");
            client.print("<form action=\"/set-timer\" method=\"GET\">");
            client.print("<label for=\"minutes\">Enter minutes:</label><br>");
            client.print("<input type=\"number\" id=\"minutes\" name=\"minutes\" min=\"1\" max=\"999\" required><br><br>");
            client.print("<input type=\"submit\" value=\"Start Timer\">");
            client.print("</form>");
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
          previousMillis = millis();
          countdownSeconds = 30 * 60;
          updateCountdown();
          displayTimer();
        }
        if (currentLine.endsWith("GET /O")){
          isTimerSet = false;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

void updateCountdown() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - previousMillis;

  if (countdownSeconds > 0 && elapsed >= interval) {
    int secondsPassed = elapsed / interval;
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
  matrix.beginText(0, 1, 0xFF00FF00);
  matrix.print(+ timeStr);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void setup(){
  Serial.begin(115200);
  while (!Serial);

  connectToWiFi();
  RTC.begin();
  Serial.println("\nStarting connection to server...");
  timeClient.begin();
  timeClient.update();

  server.begin();

  auto timeZoneOffsetHours = -7;
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  RTCTime currentTime;
  RTC.getTime(currentTime); 
  Serial.println("The RTC was just set to: " + String(currentTime));

  Serial.println("Initializing LED Matrix");
  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  const char text[] = "UNO r4";
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText();
  matrix.endDraw();
  delay(2000);
}

void loop(){
  serverSetup();
  RTCTime currentTime;

  if (isTimerSet) {
    timerLoop();
  } else {
    RTC.getTime(currentTime); 
    Serial.print(currentTime.getDayOfMonth());
    Serial.print("/");
    Serial.print(Month2int(currentTime.getMonth()));
    Serial.print("/");
    Serial.print(currentTime.getYear());
    Serial.print(" - ");
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
}
