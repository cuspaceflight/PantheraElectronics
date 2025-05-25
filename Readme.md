
# PANTHERA
Software for the Panthera electronics.

PCB contains a LORA RA-02, MPU6050, BMP280, UBLOX MAX-m8q GPS, and Teensy 4.2

The software is designed to report some data over LORA while logging as much information as possible to the internal SD card.

## Build instructions
### IDE
The Arduino IDE should be able to run and build the project

### Manual
Dependencies
```sh
arduino-cli
teensy-loader-cli
minicom
make
```

Ensure the port in the Makefile is set correctly

To build there is either
```sh
make build
make upload
```
Where upload will also upload to the teensy board.

```sh
make listen
```
Will start listening on the serial port

## TESTS
The Tests folder includes a list of files that when compiled test individual components of the PCB

These tests can be conditionally compiled by settings the relevant flags within PantheraSoftware.ino

While the tests are designed for this PCB they should be useable for any other board will small modifications

These tests ensure correct running of each of the components to track down
any broken components


## Notes
The reason behind the name srcFiles instead of src is due to arduino-cli including
any files either in the base directory, or files includes in src. This leads to
double inclusion of Main.cpp if header guards are not used which I chose to
avoid, hence the odd naming of the folder
