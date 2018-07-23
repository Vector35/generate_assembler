CPPFLAGS = 
CPPFLAGS += -I/usr/local/include -std=c++11 -O0 -g
#CPPFLAGS += -I/usr/local/include -std=c++11 -Ofast

all: assemble test

# this contains the assembler logic
asmapi.o: asmapi.cpp
	g++ $(CPPFLAGS) asmapi.cpp -c

# this is a command line utility that uses the assembler logic to assemble files
assemble: assemble.cpp asmapi.o
	g++ $(CPPFLAGS) assemble.cpp asmapi.o -lcapstone -o assemble

# this is a command line utility that tests the assembler logic
test: test.cpp asmapi.o
	g++ $(CPPFLAGS) test.cpp asmapi.o -lcapstone -o test

clean:
	rm asmapi.o 2>/dev/null
	rm assemble 2>/dev/null
	rm test 2>/dev/null

