#include "Adafruit_BMP280.h"
#include "Adafruit_MPU6050.h"
#include "Arduino.h"
#include "LoRa.h"
#include "NMEAGPS.h"
#include "SD.h"
#include "SoftwareSerial.h"
#include "TeensyThreads.h"

#include <assert.h>
#include <stdint.h>

/**
 * TODO:
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

typedef struct LoraMessage_T {
    uint32_t timestamp;
    uint32_t entry;
    float gps_lat;
    float gps_lon;
    float gps_alt;
    float gps_speed;
    float bmp_pressure;
    float bmp_temperature;
    float mpu_accel_x;
    float mpu_accel_y;
    float mpu_accel_z;
    float mpu_gyro_x;
    float mpu_gyro_y;
    float mpu_gyro_z;
    float mpu_temp;
} LoraMessage;

/**
 * Defines
 */
// #define DEBUG
// #define DEBUG_LORA

#define LORA_FREQ 433E6
#define BMP_ADDR 0x76
#define BUFFER_SIZE 255
#define GPS_RX_PIN 0
#define GPS_TX_PIN 1
#define GPS_BAUDRATE 9600
#define SD_CARD_SELECT BUILTIN_SDCARD

#ifdef DEBUG
#define SD_CARD_MAX_ENTRY 10
#else
#define SD_CARD_MAX_ENTRY 2000
#endif

void transmit_lora(const uint8_t*, size_t);

/**
 * Debug
 */

#ifdef DEBUG
#pragma message("Debug enabled")
#define DEBUG_BUFFER_SIZE 255

#ifdef DEBUG_LORA
#pragma message("Debug via LoRa")
#endif

char g_DebugBuffer[DEBUG_BUFFER_SIZE];

Threads::Mutex g_DebugMutex;

#define DEBUG_MSG_SERIAL(...)                                                                      \
    do {                                                                                           \
        snprintf(g_DebugBuffer, DEBUG_BUFFER_SIZE, __VA_ARGS__);                                   \
        Serial.print(g_DebugBuffer);                                                               \
    } while (0)

#ifdef DEBUG_LORA
#define DEBUG_MSG(...)                                                                             \
    do {                                                                                           \
        DEBUG_MSG_SERIAL(__VA_ARGS__);                                                             \
        transmit_lora(g_DebugBuffer, DEBUG_BUFFER_SIZE);                                           \
    } while (0)
#else
#define DEBUG_MSG(...) DEBUG_MSG_SERIAL(__VA_ARGS__)
#endif

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

File current_file;
bool new_file = true;

int form_message(char* buffer, size_t max_size)
{
    assert(sizeof(LoraMessage) < max_size);

    LoraMessage message;
    message.timestamp = millis();
    message.entry = g_CurrentTotalEntry;

    message.gps_lat = g_SensorData.gps.latitude();
    message.gps_lon = g_SensorData.gps.longitude();
    message.gps_alt = g_SensorData.gps.altitude();
    message.gps_speed = g_SensorData.gps.speed_mph();

    message.bmp_pressure = g_SensorData.bmp_pressure;
    message.bmp_temperature = g_SensorData.bmp_temperature;
    message.mpu_accel_x = g_SensorData.mpu_accel[0];
    message.mpu_accel_y = g_SensorData.mpu_accel[1];
    message.mpu_accel_z = g_SensorData.mpu_accel[2];
    message.mpu_gyro_x = g_SensorData.mpu_gyro[0];
    message.mpu_gyro_y = g_SensorData.mpu_gyro[1];
    message.mpu_gyro_z = g_SensorData.mpu_gyro[2];
    message.mpu_temp = g_SensorData.mpu_temp;

    memcpy(buffer, &message, sizeof(LoraMessage));

    return sizeof(LoraMessage);

    // return snprintf(buffer, max_size,
    //     "|%10ld| |%10ld| |%5.5f,%5.5f| |%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f| "
    //     "|%5.5f,%5.5f,%5.5f| |%5.5f|\n",
    //     millis(), g_CurrentTotalEntry, g_SensorData.bmp_pressure, g_SensorData.bmp_temperature,
    //     g_SensorData.mpu_accel[0], g_SensorData.mpu_accel[1], g_SensorData.mpu_accel[2],
    //     g_SensorData.mpu_gyro[0], g_SensorData.mpu_gyro[1], g_SensorData.mpu_gyro[2],
    //     g_SensorData.mpu_temp, g_SensorData.gps.latitude(), g_SensorData.gps.longitude(),
    //     g_SensorData.gps.altitude(), g_SensorData.gps.speed_mph());
}

/**
 * Transmit the message specified by data and len via the LoRa
 * module
 * If debugging also send the message via serial
 */
void transmit_lora(const char* buffer, size_t size)
{
#ifdef DEBUG
    Threads::Scope m(g_DebugMutex);
#endif
    LoRa.beginPacket();
    LoRa.write((uint8_t*)buffer, size);
    LoRa.endPacket();
}

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

    DEBUG_MSG(
        "Temperature = %5.5f *C | Pressure = %5.5f hPa\n", temp.temperature, pressure.pressure);
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

    DEBUG_MSG("AX: %5.5f, AY: %5.5f, AZ: %5.5f | GX: %5.5f, GY: %5.5f, GZ: %5.5f | T: %5.5f\n",
        a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z,
        temp.temperature);
}

/**
 * Read from the GPS serial port and transmit over LoRa
 */
void gps_read_loop()
{
    static char buffer[BUFFER_SIZE];
    while (true) {
        while (g_GPS.available(g_GPS_port)) {
            g_SensorData.gps = g_GPS.read();

            int length = form_message(buffer, BUFFER_SIZE);

            transmit_lora(buffer, length);
        }
    }
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

    // LoRa.setTxPower(20);
    // LoRa.setGain(3);
    LoRa.setSyncWord(0x34);
    LoRa.setSpreadingFactor(11);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setPreambleLength(8);
    LoRa.enableCrc();
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

    snprintf(g_Buffer, BUFFER_SIZE, "Capture_%d", count);
    SD.mkdir(g_Buffer);
    snprintf(g_DirectoryName, BUFFER_SIZE, "Capture_%d", count);

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
void init_gps()
{
    g_GPS_port.begin(GPS_BAUDRATE);
    threads.addThread(gps_read_loop);
}

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
    if (new_file) {
        new_file = false;

        int size
            = snprintf(g_Buffer, BUFFER_SIZE, "%s/Entry_%d.txt", g_DirectoryName, g_CurrentFile);
        current_file = SD.open(g_Buffer, FILE_WRITE);

        g_CurrentFileEntry = 0;
    }

    // Synchronous Reads
    read_mpu();
    read_bmp();

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
