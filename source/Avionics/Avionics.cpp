#include "Adafruit_BMP280.h"
#include "Adafruit_MPU6050.h"
#include "Arduino.h"
#include "LoRa.h"
#include "NMEAGPS.h"
#include "SD.h"
#include "SoftwareSerial.h"

#include <stdint.h>

/**
 * TODO:
 * Read GPS
 * Test
 */

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
#define SD_CARD_SELECT BUILTIN_SDCARD
#define SD_CARD_MAX_ENTRY 2000

void transmit_lora(const uint8_t*, size_t);

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
        transmit_lora(g_DebugBuffer, DEBUG_BUFFER_SIZE);                                           \
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
char g_DirectoryName[30];

int16_t g_CurrentFileEntry = 0;
int32_t g_CurrentTotalEntry = 0;
int16_t g_CurrentFile = 0;

/**
 * Transmit the message specified by data and len via the LoRa
 * module
 * If debugging also send the message via serial
 */
void transmit_lora(const char* buffer, size_t size)
{
    LoRa.beginPacket();
    LoRa.write((uint8_t*)buffer, size);
    LoRa.endPacket();
}

/**
 * Initialize the LoRa Module
 */
void init_lora()
{
    int status = LoRa.begin(LORA_FREQ);
    while (!status) {
        status = LoRa.begin(LORA_FREQ);
        // No Debug Message as not possible to transmit over LoRa at this point
        delay(100);
    }
    DEBUG_MSG("LoRa Initialized\n");

    LoRa.setTxPower(20);
    LoRa.setSpreadingFactor(10);
    LoRa.setSyncWord(0xAA);
}

/**
 * Initialize the SD card
 * Create a new directory for the flight data
 */
void init_sd_card()
{
    while (!SD.begin(SD_CARD_SELECT)) {
        DEBUG_MSG("Failed to initialize SD\n");
        delay(100);
    }

    DEBUG_MSG("SD card initialized\n");

    /* Create New Directory */

    File root = SD.open("/");
    int16_t count = 0;
    while (true) {
        File entry = root.openNextFile();
        if (!entry)
            break;
        count++;
    }

    snprintf(g_Buffer, BUFFER_SIZE, "/Capture_%d", count);
    SD.mkdir(g_Buffer);
    snprintf(g_DirectoryName, BUFFER_SIZE, "/Capture_%d", count);

    DEBUG_MSG(g_Buffer, "Created directory Capture_%d\n", count);
}

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
 * Read from the BMP using the sensors
 */
void read_bmp()
{
    sensors_event_t temp, pressure;
    g_BMP_temp_sensor->getEvent(&temp);
    g_BMP_pressure_sensor->getEvent(&pressure);

    g_SensorData.bmp_temperature = temp.temperature;
    g_SensorData.bmp_pressure = pressure.pressure;

    DEBUG_MSG("Temperature = %f *C | Pressure = %f hPa", temp.temperature, pressure.pressure);
}

/**
 * Read from the MPU using the sensors
 */
void read_mpu()
{
    sensors_event_t a, g, temp;
    g_MPU.getEvent(&a, &g, &temp);
    memcpy(&g_SensorData.mpu_accel, &a.acceleration.v, sizeof(a.acceleration));
    memcpy(&g_SensorData.mpu_gyro, &g.gyro.v, sizeof(g.gyro));
    g_SensorData.mpu_temp = temp.temperature;

    DEBUG_MSG("AX: %f, AY: %f, AZ: %f | GX: %f, GY: %f, GZ: %f | T: %f", a.acceleration.x,
        a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z, temp.temperature);
}

/**
 * Read from the GPS and transmit via LoRa the current information
 */
void read_and_transmit_gps() { }

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

    DEBUG_MSG("All Initialized\n");
}

/**
 * Called every cycle
 */
void loop()
{
    static File current_file;
    static bool new_file = true;

    if (new_file) {
        new_file = false;

        snprintf(g_Buffer, BUFFER_SIZE, "%s/Entry_%d", g_DirectoryName, g_CurrentFile);
        current_file = SD.open(g_Buffer, FILE_WRITE);
        g_CurrentFileEntry = 0;
    }

    // Synchronous Reads
    read_mpu();
    read_bmp();

    int total = snprintf(g_Buffer, BUFFER_SIZE, "|%10ld| |%10ld| |%f,%f| |%f,%f,%f,%f,%f,%f,%f|\n",
        millis(), g_CurrentTotalEntry, g_SensorData.bmp_pressure, g_SensorData.bmp_temperature,
        g_SensorData.mpu_accel[0], g_SensorData.mpu_accel[1], g_SensorData.mpu_accel[2],
        g_SensorData.mpu_gyro[0], g_SensorData.mpu_gyro[1], g_SensorData.mpu_gyro[2],
        g_SensorData.bmp_temperature);

    DEBUG_MSG(g_Buffer, total);

    current_file.print(g_Buffer);

    g_CurrentFileEntry++;
    g_CurrentTotalEntry++;

    if (g_CurrentFileEntry > SD_CARD_MAX_ENTRY) {
        new_file = true;
        g_CurrentFile++;

        current_file.flush();
        current_file.close();
    }

#ifdef DEBUG
    delay(500);
#endif
}
