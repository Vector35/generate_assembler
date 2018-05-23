/* multithreaded version of survey_opcs.cpp */

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

/******************************/
/* capstone stuff */
/******************************/
#include <capstone/capstone.h>

void disasm(uint8_t *data, char *result)
{
	/* init? */
	static thread_local csh handle = 0;
	static thread_local cs_insn *insn = NULL;
	if(insn == NULL) {
		cs_mode mode = (cs_mode)(CS_MODE_LITTLE_ENDIAN | CS_MODE_ARM);
		if(cs_open(CS_ARCH_ARM, mode, &handle) != CS_ERR_OK) {
			fprintf(stderr, "ERROR: cs_open()\n");
			exit(-1);
		}

		insn = cs_malloc(handle);
		if(!insn) {
			fprintf(stderr, "ERROR: cs_malloc()\n");
			exit(-1);
		}		
	}

	uint64_t addr = 0;
	size_t size = 4;
	const uint8_t *pinsword = data;

	size_t count = cs_disasm_iter(handle, &pinsword, &size, &addr, insn);
	if(count != 1) {
		if(cs_errno(handle) == CS_ERR_OK) {
			if(result)
				strcpy(result, "undef");
		}
		else {
			fprintf(stderr, "ERROR: cs_disasm_iter()\n");
			exit(-1);
		}
	}
	else
	if(result) {
		/* includes the .fmt */
		strcpy(result, insn->mnemonic);
	}
	else {
		fprintf(stderr, "ERROR\n");
		exit(-1);
	}
}

/******************************/
/* worker */
/******************************/

#define MODE_COUNT 0
#define MODE_SEED 1
struct worker_arg {
	int mode;
	uint64_t start;
	uint64_t stop;
	unordered_map<string,uint32_t> *result;
};

void *worker(void *arg_) {
	/* verbose */
	struct worker_arg *arg = (struct worker_arg *)arg_;
	fprintf(stderr, "worker thread [%08llX,%08llX) STARTING!\n", arg->start, arg->stop);

	/* capstone */
	csh CS_handle = 0;
	cs_insn *CS_insn = NULL;
	cs_mode mode = (cs_mode)(CS_MODE_LITTLE_ENDIAN | CS_MODE_MIPS32R6);
	if(cs_open(CS_ARCH_MIPS, mode, &CS_handle) != CS_ERR_OK) {
		fprintf(stderr, "ERROR: cs_open()\n");
		exit(-1);
	}
	CS_insn = cs_malloc(CS_handle);

	/* work */
	unordered_map<string,uint32_t> *result = arg->result;
	for(uint64_t insword64=arg->start; insword64!=arg->stop; ++insword64) {
		char distxt[64];
		uint32_t insword32 = insword64;
		disasm((uint8_t *)&insword32, distxt);

		auto it = result->find(distxt);
		if(it == result->end()) {
			if(arg->mode == MODE_SEED)
				(*result)[distxt] = insword32;
			else if(arg->mode == MODE_COUNT)
				(*result)[distxt] = 1;
		}
		else {
			if(arg->mode == MODE_COUNT)
				(*result)[distxt] += 1;
		}
	}

	fprintf(stderr, "worker thread [%08llX,%08llX) EXITING!\n", arg->start, arg->stop);
	return 0;
}

/******************************/
/* main */
/******************************/
#define NTHREADS 32
#define MAXENCODING 0x100000000
int main(int ac, char **av)
{
	int mode = MODE_SEED;
	if(ac>1 && !strcmp(av[1], "count")) {
		fprintf(stderr, "MODE: count opcodes in instruction space\n");
		mode = MODE_COUNT;
	}
	else if(ac>1 && !strcmp(av[1], "seed")) {
		mode = MODE_SEED;
		fprintf(stderr, "MODE: remember example instruction word for each opcode\n");
	}

	pthread_t threads[NTHREADS];
	struct worker_arg args[NTHREADS];
	unordered_map<string,uint32_t> results[NTHREADS];

	/* initiate threads */
	uint32_t work_per_thread = (uint64_t)MAXENCODING / NTHREADS;
	for(int i=0; i<NTHREADS; ++i) {
		/* set the start of work */
		args[i].mode = mode;
		args[i].start = i*work_per_thread;
		/* set the end of work */
		if(i == NTHREADS-1)
			args[i].stop = MAXENCODING;
		else
			args[i].stop = args[i].start + work_per_thread;
		/* set where results shoudl be stored */
		args[i].result = &(results[i]);

		pthread_create(&(threads[i]), NULL, worker, &(args[i]));
	}

	/* join all threads */
	for(int i=0; i<NTHREADS; ++i)
		pthread_join(threads[i], NULL);

	/* join all results */
	unordered_map<string,uint32_t> result;
	for(int i=0; i<NTHREADS; ++i) {
		for(auto it = results[i].begin(); it != results[i].end(); it++) {

			if(result.find(it->first) == result.end()) {
				result[it->first] = it->second;

				if(mode == MODE_SEED)
					printf("\"%s\": 0x%08X\n", (it->first).c_str(), it->second);
				else
				if(mode == MODE_COUNT)
					printf("\"%s\"': %d\n", (it->first).c_str(), it->second);
			}
		}
	}
}

