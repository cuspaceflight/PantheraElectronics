#include <LoRa.h>
#include <SPI.h>

int counter = 0;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(100);

    int status = LoRa.begin(9600);
    if (!status) {
        Serial.println("LoRa failed");
        status = LoRa.begin(9600);
        delay(1000);
    }
}

void loop()
{
    Serial.print("Sending packet: ");
    Serial.println(counter);

    LoRa.beginPacket();
    LoRa.print("Hello: ");
    LoRa.print(counter);
    LoRa.endPacket();
    counter++;
    delay(1000);
}
