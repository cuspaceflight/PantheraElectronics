import sys
import serial

import time
import random

import numpy
import matplotlib.pyplot as plt

serialPort = serial.Serial(
    port=sys.argv[1], baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

fig, ax = plt.subplots(1, 1);

serialString = ""
with open("Output", "w") as file:
    while True:
        serialString = serialPort.readline()

        try:
            decoded = serialString.decode("Ascii")
            decoded = exampleString
            print(decoded)

            sections = [x.split(",") for x in decoded.split("|") if x != " " and x != "" and x != "\n"]
            # print(sections)

            time_ms = int(sections[0][0])
            index = int(sections[1][0])

            bmp = sections[2]
            bmp_pressure = bmp[0]
            bmp_temp = bmp[1]

            mpu = sections[3]
            mpu_accel = mpu[0:3]
            mpu_gyro = mpu[3:-2]
            mpu_temp = mpu[-1]

            gps = sections[4]
            lat = gps[0]
            lon = gps[1]
            alt = float(gps[2])

            speed = sections[5][0]

            file.write(f"{time_ms},{index},{bmp_pressure},{bmp_temp},{mpu_accel[0]},{mpu_accel[1]},{mpu_accel[2]},{mpu_gyro[0]},{mpu_gyro[1]},{mpu_gyro[2]},{mpu_temp},{lat},{lon},{alt},{speed}")
            file.flush()

            plt.scatter(time_ms/1000, alt)

            plt.pause(0.05)

        except:
            print("Fail")
