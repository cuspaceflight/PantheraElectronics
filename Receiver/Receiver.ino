#include "Arduino.h"

  UART gps(4, 5, NC, NC);

void setup()
{
    Serial.begin(9600);
    gps.begin(9600);
}

void loop()
{
    while (gps.available()) {
        Serial.print((char)gps.read());
    }
}
