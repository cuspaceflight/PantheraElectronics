#include "Arduino.h"

// UART gps(4, 5, NC, NC);

char buffer[255];
void setup()
{
    Serial.begin(9600);
    // gps.begin(9600);

    snprintf(buffer, 255,
            "|%10ld| |%10ld| |%f,%f| |%f,%f,%f,%f,%f,%f,%f| |%f,%f,%f| |%f|\n", millis(),
            100, 1024.5f, 24.0f,
            0.0f, 0.1f, 0.2f,
            0.3f, 0.4f, 0.5f,
            25.0f, 100.0f, 30.0f,
            200.0f, 200.0f);
}

void loop()
{
    // while (gps.available()) {
    //     Serial.print((char)gps.read());
    // }
    Serial.print(buffer);
    delay(1000);
}
