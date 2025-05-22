#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <Wire.h>

Adafruit_BMP280 bmp(&Wire1);
Adafruit_Sensor* bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor* bmp_pressure = bmp.getPressureSensor();

void setup_code()
{
    Serial.begin(9600);
    while (!Serial)
        delay(100);

    int32_t status = bmp.begin(0x76);
    while (!status) {
        Serial.print("Failed to initialize BMP: Sensor ID:");
        Serial.println((uint16_t)bmp.sensorID());
        status = bmp.begin(0x76);
        delay(100);
    }

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_500);
}

void loop_code()
{
    sensors_event_t temp, pressure;
    bmp_temp->getEvent(&temp);
    bmp_pressure->getEvent(&pressure);

    char buffer[200];
    sprintf(buffer, "Temperature = %f *C | Pressure = %f hPa", temp.temperature, pressure.pressure);
    Serial.println(buffer);
    delay(1000);
}
