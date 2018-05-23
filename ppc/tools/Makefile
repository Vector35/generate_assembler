CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -Ofast -g

all: gofer.so survey_opcs

gofer.so: gofer.cpp
	g++ $(CPPFLAGS) \
		-lcapstone \
		-shared -o gofer.so gofer.cpp \
		-Wl,-headerpad_max_install_names
	install_name_tool -change libcapstone.3.dylib /usr/local/lib/libcapstone.3.dylib gofer.so
	#install_name_tool -add_rpath `pwd` gofer.so

survey_opcs: survey_opcs.cpp
	g++ -std=c++11 -Ofast survey_opcs.cpp -lcapstone -o survey_opcs

clean:
	rm survey_opcs 2>/dev/null
	rm gofer.so 2>/dev/null
