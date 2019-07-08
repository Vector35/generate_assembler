CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g

all: gofer.so opc_seeds

gofer.so: gofer.cpp
	g++ $(CPPFLAGS) \
		-shared -o gofer.so gofer.cpp \
		-Wl,-headerpad_max_install_names \
		-I$(HOME)/repos/vector35/binaryninja/api \
		$(HOME)/repos/vector35/binaryninja/core/libbinaryninjacore.dylib
	#	$(HOME)/repos/vector35/binaryninja/ui/shared/libbinaryninjaui.dylib
	#install_name_tool -change libbinaryninjacore.dylib  gofer.so
	#install_name_tool -add_rpath `pwd` gofer.so

opc_seeds: opc_seeds.cpp gofer.so
	g++ -std=c++11 -Ofast -Wl,-headerpad_max_install_names opc_seeds.cpp -pthread gofer.so -o opc_seeds
	install_name_tool -add_rpath `pwd` opc_seeds
	install_name_tool -add_rpath $(HOME)/repos/vector35/binaryninja/core/ gofer.so

clean:
	rm gofer.so 2>/dev/null
	rm opc_seeds 2>/dev/null
	rm gofer 2>/dev/null
	rm -rf gofer.so.dSYM 2>/dev/null
	rm *.pyc 2>/dev/null
