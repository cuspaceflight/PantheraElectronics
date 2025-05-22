#include <LoRa.h>
#include <SPI.h>

int counter = 0;

#define LORA_FREQ 433E6

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(100);

    int status = LoRa.begin(LORA_FREQ);
    if (!status) {
        Serial.println("LoRa failed");
        status = LoRa.begin(LORA_FREQ);
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
