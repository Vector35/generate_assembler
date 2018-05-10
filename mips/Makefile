CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g

all: gofer.so

gofer.so: gofer.cpp
	g++ $(CPPFLAGS) \
		-lcapstone \
		-shared -o gofer.so gofer.cpp \
		-Wl,-headerpad_max_install_names
	install_name_tool -change libcapstone.3.dylib /usr/local/lib/libcapstone.3.dylib gofer.so
	#install_name_tool -add_rpath `pwd` gofer.so

clean:
	rm gofer.so 2>/dev/null
