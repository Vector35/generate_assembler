/* multithreaded version of survey_opcs.cpp */
#define NTHREADS 16
#define MAXENCODING 0x100000000 /* top nybble is condition field (0000b == 'eq') */
								/* CANCEL THAT! for some "v" instructions they are extra */
#define MODE_COUNT 0
#define MODE_SEED 1
#define MODE_VERBOSE 1

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

/* from gofer.so */
extern "C" void get_disasm_binja(uint8_t *data, char *);

/******************************/
/* worker */
/******************************/

struct worker_arg {
	int mode;
	uint64_t start;
	uint64_t stop;
	unordered_map<string,uint32_t> *result;
};

void replace_cond_suffix(char *target)
{
	#define NCCS 16
	const char *cond_suffixes[NCCS] = {
		"eq",		/* equal */
		"ne",		/* not equal */
		"cs", "hs",	/* carry set or unsigned higher or same */
		"cc", "lo",	/* carry clear or unsigned lower */
		"mi",		/* negative */
		"pl",		/* positive or zero */
		"vs",		/* ... */
		"vc",
		"hi",
		"ls",
		"ge",
		"lt",
		"gt",
		"le"
	};
	
	for(int i=0; i<NCCS; ++i) {
		if(0==strncmp(target, cond_suffixes[i], 2)) {
			char tmp[256];
			strcpy(tmp, target+2);
			strcpy(target, "<c>");
			strcpy(target+3, tmp);
			break;
		}
	}	
}

string opcode_get(char *distxt)
{
	char opcodeA[256]="error";

	/* copy disassembly, end at first space */
	strcpy(opcodeA, distxt);
	for(int i=0; opcodeA[i]; ++i) {
		if(opcodeA[i] == ' ') {
			opcodeA[i] = '\0';
			break;
		}
	}

	return opcodeA;

	/* replace conditional suffix at end, eg "vcvtbeq.f32.f16" -> "vcvtb<c>.f32.f16" */
	int lenA = strlen(opcodeA);
	if(lenA > 2)
		replace_cond_suffix(opcodeA + lenA - 2);

	/* "vcvtteq.f32.f16" -> "vcvtt<c>.f32.f16" */
	if(lenA > 10 && opcodeA[lenA-4] == '.' && opcodeA[lenA-8] == '.') {
		replace_cond_suffix(opcodeA + lenA - 10);
	}

	/* "vnmlseq.f32" -> "vnmls<c>.f32" */
	if(lenA > 6 && opcodeA[lenA - 4] == '.' && isdigit(opcodeA[lenA - 2]) && isdigit(opcodeA[lenA - 1])) {
		replace_cond_suffix(opcodeA + lenA - 6);
	}

	/* "vmoveq.32" -> "vmov<c>.32" */
	/* "vmoveq.s8" -> "vmov<c>.s8" */
	/* "vmoveq.u8" -> "vmov<c>.u8" */
	if(lenA > 5 && opcodeA[lenA - 3] == '.' && isdigit(opcodeA[lenA - 1])) {
		replace_cond_suffix(opcodeA + lenA - 5);
	}

	/* blah blah "vmoveq.8" -> "vmov<c>.8" */
	if(lenA > 4 && opcodeA[lenA - 2] == '.' && isdigit(opcodeA[lenA - 1])) {
		replace_cond_suffix(opcodeA + lenA - 4);
	}

	//printf("converted \"%s\" to \"%s\"\n", distxt, result.c_str());
	return opcodeA;
}

void *worker(void *arg_)
{
	/* verbose */
	struct worker_arg *arg = (struct worker_arg *)arg_;
	fprintf(stderr, "worker thread [%08llX,%08llX) STARTING!\n", arg->start, arg->stop);

	/* work */
	unordered_map<string,uint32_t> *result = arg->result;
	for(uint64_t insword64=arg->start; insword64!=arg->stop; ++insword64) {
		char distxt[256];
		uint32_t insword32 = insword64;

		get_disasm_binja((unsigned char *)&insword32, distxt);
		string opcode = opcode_get(distxt);

		auto it = result->find(opcode);
		if(it == result->end()) {
			if(MODE_VERBOSE) {
				printf("new dude: %08X: %s\n", insword32, opcode.c_str());
			}

			if(arg->mode == MODE_SEED)
				(*result)[opcode] = insword32;
			else if(arg->mode == MODE_COUNT)
				(*result)[opcode] = 1;
		}
		else {
			if(arg->mode == MODE_COUNT)
				(*result)[opcode] += 1;
		}
	}

	fprintf(stderr, "worker thread [%08llX,%08llX) EXITING!\n", arg->start, arg->stop);
	return 0;
}

/******************************/
/* main */
/******************************/
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
	uint64_t work_per_thread = (NTHREADS == 1) ? MAXENCODING-1 : MAXENCODING / NTHREADS;

	printf("work per thread: %llu\n", work_per_thread);
	for(int i=0; i<NTHREADS; ++i) {
		/* set the start of work */
		args[i].mode = mode;
		args[i].start = i*work_per_thread;
		/* set the end of work */
		if(i == NTHREADS-1)
			args[i].stop = MAXENCODING;
		else
			args[i].stop = args[i].start + work_per_thread;
		/* set where results should be stored */
		args[i].result = &(results[i]);

		pthread_create(&(threads[i]), NULL, worker, &(args[i]));

		if(i == 0) {
			printf("waiting for first thread to init binja\n");
			sleep(2);
		}
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

