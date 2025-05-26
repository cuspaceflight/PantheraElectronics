#include <SPI.h>
#include <LoRa.h>

#define LORA_FREQ 433E6

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("LoRa Receiver");

    LoRa.setSPI(SPI);
    while (!LoRa.begin(LORA_FREQ)) {
        Serial.println("Starting LoRa failed!");
        delay(100);
    }
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Received packet '");

        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }

        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
    }
}
