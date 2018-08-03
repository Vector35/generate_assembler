CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g

all: gofer.so opc_seeds

gofer.so: gofer.cpp
	g++ $(CPPFLAGS) \
		-lcapstone -lbfd -liberty -lopcodes -lz \
		-shared -o gofer.so gofer.cpp \
		-Wl,-headerpad_max_install_names
	install_name_tool -change libcapstone.3.dylib /usr/local/lib/libcapstone.3.dylib gofer.so
	#install_name_tool -add_rpath `pwd` gofer.so

opc_seeds: opc_seeds.cpp
	g++ -std=c++11 -Ofast opc_seeds.cpp -pthread -lcapstone -o opc_seeds

clean:
	rm gofer.so 2>/dev/null
	rm opc_seeds 2>/dev/null
	rm gofer 2>/dev/null
	rm -rf gofer.so.dSYM 2>/dev/null
