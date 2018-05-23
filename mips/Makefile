#CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g
CPPFLAGS += -I/usr/local/include -std=c++11 -Ofast

all: assemble

assemble: assemble.cpp common.o table.o
	g++ $(CPPFLAGS) assemble.cpp common.o table.o -lcapstone -o assemble

common.o: common.cpp
	g++ $(CPPFLAGS) -c common.cpp -o common.o

table.o: table.cpp
	g++ $(CPPFLAGS) -c table.cpp -o table.o

clean:
	rm *.o 2>&1
	rm assemble 2>&1
	rm -rf assemble.dSYM 2>&1
