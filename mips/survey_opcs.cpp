#include <stdio.h>
#include <time.h>

#include <signal.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>
using namespace std;

/******************************/
/* reporting/signal stuff */
/******************************/
volatile sig_atomic_t show_status = false;
void cb_alarm(int sig) {
	show_status = true;
}

volatile sig_atomic_t interrupt = false;
void cb_int(int sig) {
	interrupt = true;
}

/******************************/
/* capstone stuff */
/******************************/
#include <capstone/capstone.h>
#include <capstone/arm.h>

csh		CS_handle	= 0;
cs_insn	*CS_insn	= NULL;

void CS_init(void)
{
	int rc = -1;

	cs_mode mode = (cs_mode)(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32R6);
	if(cs_open(CS_ARCH_MIPS, mode, &CS_handle) != CS_ERR_OK) {
		printf("ERROR: cs_open()\n");
		exit(-1);
	}

	CS_insn = cs_malloc(CS_handle);
}

void CS_disasm(uint8_t *data, char *result)
{
	uint64_t addr = 0;
	size_t size = 4;
	const uint8_t *pinsword = data;

	size_t count = cs_disasm_iter(CS_handle, &pinsword, &size, &addr, CS_insn);
	if(count != 1) {
		if(cs_errno(CS_handle) == CS_ERR_OK) {
			if(result)
				strcpy(result, "undef");
		}
		else {
			printf("ERROR: cs_disasm_iter()\n");
			exit(-1);
		}
	}
	else
	if(result) {
		strcpy(result, CS_insn->mnemonic);
	}
	else {
		printf("ERROR\n");
		exit(-1);
	}
}

/******************************/
/* main */
/******************************/
int main(int ac, char **av)
{
	map<string,uint32_t> data;

	#define MODE_COUNT 0				// count the number of times each opcode is encountered
	#define MODE_SEED 1					// store an instruction word for each opcode
	int mode = MODE_COUNT;
	if(!strcmp(av[1], "count")) {
		fprintf(stderr, "MODE: count opcodes in instruction space\n");
		mode = MODE_COUNT;
	}
	else if(!strcmp(av[1], "seed")) {
		mode = MODE_SEED;
		fprintf(stderr, "MODE: remember example instruction word for each opcode\n");
	}

	double delta;
	struct timespec t0,t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);

	signal(SIGALRM, cb_alarm);
	signal(SIGINT, cb_int);
	alarm(4);

	CS_init();

	for(uint32_t insword=0; ; ++insword) {
		char result[64];

		CS_disasm((uint8_t *)&insword, result);

		if(data.find(result) == data.end()) {
			if(mode == MODE_COUNT)
				data[result] = 1;
			else if(mode == MODE_SEED)
				data[result] = insword;
		}
		else {
			if(mode == MODE_COUNT)
				data[result] += 1;
		}

		if(show_status) {
			clock_gettime(CLOCK_MONOTONIC, &t1);
			delta = (double)(t1.tv_nsec - t0.tv_nsec) / 1000000000.0;
			delta += (double)t1.tv_sec - t0.tv_sec;

			double ips = insword / delta;
			double remaining = (0x100000000 - insword) * (1/ips);

			printf("insword:%08X elapsed:%d ips:%d remaining:%dm\n",
				insword, (int)delta, (int)ips, (int)(remaining/60));

			show_status = false;
			alarm(4);
		}

		if(interrupt) {
			break;
		}

		if(insword == 0xFFFFFFFF)
			break;
	}

	for(auto it=data.begin(); it!=data.end(); it++) {
		string opc = it->first;
		uint32_t value = it->second;
		if(mode == MODE_COUNT)
			printf("\"%s\":%d\n", opc.c_str(), value);
		else if(mode == MODE_SEED)
			printf("\"%s\":0x%08X\n", opc.c_str(), value);
	}
}

