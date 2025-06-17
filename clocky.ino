#include "Arduino_LED_Matrix.h"
#include <RTC.h>

ArduinoLEDMatrix matrix;
RTC rtc;

void setup() {
  rtc.begin();
  matrix.begin();
}

void loop() {
  // Get current time
  int h = rtc.getHours();
  int m = rtc.getMinutes();

  // Format time as HH:MM
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", h, m);

  // Display time
  matrix.beginDraw();
  matrix.stroke(255);
  matrix.textScrollSpeed(100); // optional
  matrix.textFont(Font_5x7);
  matrix.text(timeStr, 0, 0);
  matrix.endDraw();

  delay(60000); // update every minute
}
