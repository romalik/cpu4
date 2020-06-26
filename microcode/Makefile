CXX=g++


all: cpu4mc_compiler


cpu4mc_compiler: mc_compiler.cpp
	$(CXX) -std=c++17 -o cpu4mc_compiler mc_compiler.cpp


install: all
	install -d /opt/cpu4/bin
	install -m 755 cpu4mc_compiler /opt/cpu4/bin/

clean:
	-rm cpu4mc_compiler

