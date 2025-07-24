import sys
import serial

serialPort = serial.Serial(
    port=sys.argv[1], baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

serialString = ""
with open("Output", "w") as file:
    while True:
        serialString = serialPort.readline()

        try:
            decoded = serialString.decode("Ascii")
            print(decoded)

            sections = [x.split(",") for x in decoded.split("|") if x != " " and x != "" and x != "\n"]
            print(sections)

            time = int(sections[0][0])
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
            alt = gps[2]

            speed = sections[5][0]

            print(f"{time} {index} {bmp_pressure} {bmp_temp} {mpu_accel} {mpu_gyro} {gps} {speed}")
            file.write(f"{time},{index},{bmp_pressure}\n")
            file.flush()

        except:
            print("Fail")
