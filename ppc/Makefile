CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g
#CPPFLAGS += -I/usr/local/include -std=c++11 -Ofast

all: assemble

assemble: assemble.cpp
	g++ $(CPPFLAGS) assemble.cpp -lcapstone -o assemble

clean:
	rm assemble 2>/dev/null

