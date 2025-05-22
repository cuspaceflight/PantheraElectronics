board=teensy:avr:teensy41
project=PantheraSoftware.ino
mcu=TEENSY41
port=/dev/ttyACM0
buildpath=Build

build ${buildpath}/${project}.hex:
	arduino-cli compile -b ${board} --build-path Build -j 0 ${project}

upload: build
	teensy-loader-cli --mcu=${mcu} -w Build/PantheraSoftware.ino.hex -v

listen:
	minicom -b 9600 -o -D ${port}

clean:
	rm -r Build
