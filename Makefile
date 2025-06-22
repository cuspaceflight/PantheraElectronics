# Teensy Settings
teensy_board=teensy:avr:teensy41
teensy_mcu=TEENSY41
teensy_port=/dev/ttyACM0

teensy_upload = teensy-loader-cli
teensy_upload_flags = --mcu=$(teensy_mcu) -v -w

# Pico  settings
pico_board=arduino:mbed_rp2040:pico
pico_port=/dev/ttyACM0
pico_destination=/run/media/aaron/RPI-RP2/

receiver_dir=Receiver
receiver_project=Receiver.ino

avionics_dir=Avionics
avionics_project=Avionics.ino

buildpath=Build

cli_flags = -j 0 --build-path $(buildpath)
compile = arduino-cli compile

CWD=$(shell pwd)

$(buildpath)/$(avionics_project).hex: $(avionics_dir)/$(avionics_project) $(buildpath) $(wildcard source/Avionics/*.cpp)
	$(compile) $(cli_flags) -b $(teensy_board) $<

$(buildpath)/$(receiver_project).uf2: $(receiver_dir)/$(receiver_project) $(buildpath) $(wildcard source/Receiver/*.cpp)
	$(compile) $(cli_flags) -b $(pico_board) $<

build_avionics: $(buildpath) $(buildpath)/$(avionics_project).hex
	@echo "Built avionics"

build_receiver: $(buildpath) $(buildpath)/$(receiver_project).uf2
	@echo "Built receiver"

upload_receiver: $(build_receiver)
	@cp $(buildpath)/$(receiver_project).uf2 $(pico_destination)
	@echo "Uploaded code to pico"

upload_avionics: $(build_avionics)
	@$(teensy_upload) $(teensy_upload_flags) Build/Avionics.ino.hex
	@echo "Uploaded code to teensy"

listen:
	minicom -b 9600 -o -D $(pico_port)

build: build_avionics build_receiver


clean:
	rm -rf Build

$(buildpath)/sketch/Tests: FORCE
	@rm -rf $(buildpath)/sketch/Tests
	@cp -rf $(CWD)/Tests $(buildpath)/sketch/Tests

$(buildpath)/sketch/source: FORCE
	@rm -rf $(buildpath)/sketch/source
	@cp -rf $(CWD)/source $(buildpath)/sketch/source

$(buildpath)/sketch:
	@mkdir -p $(buildpath)/sketch

$(buildpath): $(buildpath)/sketch $(buildpath)/sketch/source $(buildpath)/sketch/Tests

FORCE: ;
