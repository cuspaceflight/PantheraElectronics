#include "Adafruit_BMP280.h"
#include "Adafruit_MPU6050.h"
#include "Arduino.h"
#include "LoRa.h"
#include "NMEAGPS.h"
#include "SoftwareSerial.h"

#include <stdint.h>

/**
 * Structs
 */
typedef struct SensorData_T {
    float bmp_temperature;
    float bmp_pressure;

    float mpu_accel[3];
    float mpu_gyro[3];
    float mpu_temp;

    gps_fix gps;
} SensorData;

/**
 * Defines
 */
#define LORA_FREQ 433E6
#define BMP_ADDR 0x76
#define BUFFER_SIZE 255
#define GPS_RX_PIN 0
#define GPS_TX_PIN 0
#define GPS_BAUDRATE 9600

/**
 * Debug
 */
#define DEBUG
#ifdef DEBUG
#define DEBUG_BUFFER_SIZE 255
char g_DebugBuffer[DEBUG_BUFFER_SIZE];

#define DEBUG_MSG(...)                                                                             \
    do {                                                                                           \
        snprintf(g_DebugBuffer, DEBUG_BUFFER_SIZE, __VA_ARGS__);                                   \
        Serial.print(g_DebugBuffer);                                                               \
    } while (0)
#else
#define DEBUG_MSG(...)
#endif

/**
 * GLOBALS
 */
Adafruit_BMP280 g_BMP(&Wire1);
Adafruit_Sensor* g_BMP_temp_sensor = g_BMP.getTemperatureSensor();
Adafruit_Sensor* g_BMP_pressure_sensor = g_BMP.getPressureSensor();

Adafruit_MPU6050 g_MPU;

NMEAGPS g_GPS;
SoftwareSerial g_GPS_port(GPS_RX_PIN, GPS_TX_PIN);

SensorData g_SensorData;
char g_Buffer[BUFFER_SIZE];

/**
 * Initialize the LoRa Module
 */
void init_lora()
{
    int status = LoRa.begin(LORA_FREQ);
    while (!status) {
        status = LoRa.begin(LORA_FREQ);
        DEBUG_MSG("Failed to start LoRa. Retrying\n");
        delay(100);
    }
    DEBUG_MSG("LoRa Initialized\n");
}

/**
 * Initialize the SD card
 * Create a new directory for the flight data
 */
void init_sd_card() { }

/**
 * Initialize the BMP sensor
 */
void init_bmp()
{
    int status = g_BMP.begin(BMP_ADDR);
    while (!status) {
        DEBUG_MSG("Failed to start BMP. Retrying\n");
        status = g_BMP.begin(BMP_ADDR);
        delay(100);
    }
    DEBUG_MSG("Started BMP\n");

    g_BMP.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_500);
}

/**
 * Initialize the MPU sensor
 */
void init_mpu()
{
    int status = g_MPU.begin();
    while (!status) {
        DEBUG_MSG("Failed to start MPU. Retrying\n");
        status = g_MPU.begin();
        delay(100);
    }

    DEBUG_MSG("Started MPU\n");

    g_MPU.setAccelerometerRange(MPU6050_RANGE_16_G);
    g_MPU.setGyroRange(MPU6050_RANGE_1000_DEG);
    g_MPU.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

/**
 * Initialize the GPS sensor
 */
void init_gps() { g_GPS_port.begin(GPS_BAUDRATE); }

/**
 * Transmit the message specified by data and len via the LoRa
 * module
 * If debugging also send the message via serial
 */
void transmit(char* data, uint8_t len) { }

/**
 * Called once on startup
 */
void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    while (!Serial)
        ;
#endif
    init_lora();
    init_sd_card();
    init_bmp();
    init_mpu();
    init_gps();
}

/**
 * Called every cycle
 */
void loop()
{
    {
        sensors_event_t temp, pressure;
        g_BMP_temp_sensor->getEvent(&temp);
        g_BMP_pressure_sensor->getEvent(&pressure);

        snprintf(g_Buffer, BUFFER_SIZE, "Temperature = %f *C | Pressure = %f hPa", temp.temperature,
            pressure.pressure);
        Serial.println(g_Buffer);
    }

    {
        sensors_event_t a, g, temp;
        g_MPU.getEvent(&a, &g, &temp);
        snprintf(g_Buffer, BUFFER_SIZE, "AX: %f, AY: %f, AZ: %f | GX: %f, GY: %f, GZ: %f | T: %f",
            a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z,
            temp.temperature);
        Serial.println(g_Buffer);
    }

    delay(100);
}
