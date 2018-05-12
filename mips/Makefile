CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g
CPPFLAGS2 += -I/usr/local/include -std=c++11 -Ofast

all: assemble

assemble: assemble.cpp
	g++ $(CPPFLAGS) assemble.cpp -lcapstone -o assemble
