CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g

all: opc_seeds16 opc_seeds32 gofer.so

common.o: common.cpp
	g++ -std=c++11 -Ofast -c common.cpp -o common.o

opc_seeds16: opc_seeds16.cpp common.o
	g++ -std=c++11 -Ofast opc_seeds16.cpp -o opc_seeds16 -lcapstone common.o

opc_seeds32: opc_seeds32.cpp common.o
	g++ -std=c++11 -Ofast opc_seeds32.cpp -o opc_seeds32 -lcapstone common.o

gofer.so: gofer.cpp
	g++ $(CPPFLAGS) \
		-lcapstone \
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
	rm *.pyc 2>/dev/null
