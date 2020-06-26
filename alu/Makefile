CXX=g++


all: alu_firmware_generator


alu_firmware_generator: generate_alu_table.cpp
	$(CXX) -std=c++17 -o alu_firmware_generator generate_alu_table.cpp


install: all
	install -d /opt/cpu4/bin
	install -m 755 alu_firmware_generator /opt/cpu4/bin/

clean:
	-rm alu_firmware_generator

