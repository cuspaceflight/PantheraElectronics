#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        delay(100);

    int status = mpu.begin();
    while (!status) {
        Serial.println("Failed to start MPU");
        status = mpu.begin();
        delay(100);
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_1000_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("Started MPU");
}

void loop()
{
    static char buffer[200];
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    sprintf(buffer, "ACCEL (m/s^2) X: %f | Y: %f | Z: %f", a.acceleration.x, a.acceleration.y,
        a.acceleration.z);
    Serial.println(buffer);
    sprintf(buffer, "ROT (rad/s) X: %f | Y: %f | Z: %f", g.gyro.x, g.gyro.y, g.gyro.z);
    Serial.println(buffer);
    sprintf(buffer, "TEMP (degC): %f", temp.temperature);
    Serial.println(buffer);

    delay(100);
}
