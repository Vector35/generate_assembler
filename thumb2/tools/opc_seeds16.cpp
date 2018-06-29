// g++ -std=c++11 -Ofast opc_seeds.cpp -o opc_seeds -lcapstone -pthread

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

#include "common.h"

/******************************/
/* main */
/******************************/

int main(int ac, char **av)
{
	unordered_map<string, uint32_t> str2enc;
	unordered_map<string, int> str2cnt;

	for(uint32_t iw=0; iw<0x10000; ++iw) {
		char distxt[64];
		int size = disasm((uint8_t *)&iw, distxt);

		if(size != 2)
			continue;

		if(str2enc.find(distxt) != str2enc.end()) {
			str2cnt[distxt] += 1;
		}
		else {
			str2enc[distxt] = iw;
			str2cnt[distxt] = 1;
		}
	}

	int total = 0;
	for(auto it = str2enc.begin(); it != str2enc.end(); it++) {
		string opc = it->first;
		uint16_t enc = it->second;
		int amt = str2cnt[opc];
		printf("\"%s\": \"\\x%02x\\x%02X\" // %d total\n", opc.c_str(), enc & 0xFF, enc >> 8, amt);

		total += amt;
	}

	printf("%d total\n", total);
}

