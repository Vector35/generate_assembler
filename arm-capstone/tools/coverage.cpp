/* count valid disassemblies

compile:
	g++ -std=c++11 coverage.cpp -pthread -lcapstone -o coverage
*/

#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>

/******************************/
/* capstone stuff */
/******************************/
#include <capstone/capstone.h>

bool disasm(uint8_t *data)
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
	return count==1;
}

/******************************/
/* worker */
/******************************/

struct worker_arg {
	uint64_t start;
	uint64_t stop;
	uint32_t count;
};

void *worker(void *arg_) {
	/* verbose */
	struct worker_arg *arg = (struct worker_arg *)arg_;
	fprintf(stderr, "worker thread [%" PRIx64 " ,%" PRIx64 ") STARTING!\n", arg->start, arg->stop);

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
	arg->count = 0;
	for(uint64_t insword64=arg->start; insword64!=arg->stop; ++insword64) {
		uint32_t insword32 = insword64;
		if(disasm((uint8_t *)&insword32)) {
			arg->count += 1;
		}
	}

	fprintf(stderr, "worker thread [%" PRIx64 ", %" PRIx64 ") EXITING! count: %u\n", arg->start, arg->stop, arg->count);
	return 0;
}

/******************************/
/* main */
/******************************/
#define NTHREADS 32
#define MAXENCODING 0x100000000
int main(int ac, char **av)
{
	pthread_t threads[NTHREADS];
	struct worker_arg args[NTHREADS];

	/* initiate threads */
	uint32_t work_per_thread = (uint64_t)MAXENCODING / NTHREADS;
	for(int i=0; i<NTHREADS; ++i) {
		args[i].start = i*work_per_thread;
		args[i].stop = MAXENCODING;
		pthread_create(&(threads[i]), NULL, worker, &(args[i]));
	}

	/* join all threads */
	for(int i=0; i<NTHREADS; ++i)
		pthread_join(threads[i], NULL);

	/* sum results */
	uint32_t total = 0;
	for(int i=0; i<NTHREADS; ++i)
		total += args[i].count;
	printf("total: %u\n", total);
}
