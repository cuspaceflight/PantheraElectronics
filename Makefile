board_name=teensy:avr:teensy41
project=.
mcu=TEENSY41
port=/dev/ttyACM0
build:
	arduino-cli compile --fqbn ${board_name} ${project} --output-dir Build -j 0

upload: build
	teensy-loader-cli --mcu=${mcu} -w Build/PantheraSoftware.ino.hex -v

listen:
	minicom -b 9600 -o -D ${port}
