// g++ -std=c++11 -Ofast opc_seeds.cpp -o opc_seeds -lcapstone -pthread

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>

#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

#include "common.h"

volatile sig_atomic_t show_status = false;
void cb_alarm(int sig) {
	show_status = true;
}

volatile sig_atomic_t breakout = false;
void cb_int(int sig) {
	breakout = true;
}

/******************************/
/* main */
/******************************/

int main(int ac, char **av)
{
	signal(SIGALRM, cb_alarm);
	signal(SIGINT, cb_int);
	alarm(8);

	unordered_map<string, uint32_t> str2enc;
	unordered_map<string, int> str2cnt;

	for(uint64_t iw=0x0; iw<0x100000000; ++iw) {
		char distxt[64];
		int size = disasm((uint8_t *)&iw, distxt);

		if(size != 4)
			continue;
		
		if(str2enc.find(distxt) != str2enc.end()) {
			str2cnt[distxt] += 1;
		}
		else {
			str2enc[distxt] = iw;
			str2cnt[distxt] = 1;
		}

		if(show_status) {
			printf("current instruction word: %08llX\n", iw);
			show_status = false;
			alarm(8);
		}

		if(breakout)
			break;
	}

	int total = 0;
	for(auto it = str2enc.begin(); it != str2enc.end(); it++) {
		string opc = it->first;
		uint32_t enc = it->second;
		int amt = str2cnt[opc];
		printf("\"%s\": \"\\x%02x\\x%02X\\x%02X\\x%02X\" // %d total\n", opc.c_str(),
			enc & 0xFF, (enc & 0xFF00)>>8, (enc & 0xFF0000)>>16, (enc & 0xFF000000)>>24, amt);

		total += amt;
	}

	printf("%d total\n", total);
}

