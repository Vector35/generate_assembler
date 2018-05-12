/* a shared object that can be called easily with python+ctypes */

/* */
#include <stdint.h>
#include <string.h>

/* c++ stuff */
#include <map>
#include <string>
#include <vector>
#include <iostream>

/* capstone stuff */
#include <capstone/capstone.h>
#include <capstone/arm.h>

extern "C" void get_disasm_capstone(uint8_t *data, int len, char *result)
{
	int rc = -1;

	/* one-time capstone init stuff */
	static bool init = false;
	static csh handle = 0;
	static cs_insn *insn = NULL;
	if (!init) {
		cs_mode mode = (cs_mode)(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32R6);
		if(cs_open(CS_ARCH_MIPS, mode, &handle) != CS_ERR_OK) {
			printf("ERROR: cs_open()\n");
			exit(-1);
		}
		insn = cs_malloc(handle);
		init = true;
	}

	/* actually disassemble */
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
			printf("ERROR: cs_disasm_iter()\n");
			exit(-1);
		}
	}
	else
	if(result) {
		strcpy(result, insn->mnemonic);
		if(insn->op_str[0]) {
			strcat(result, " ");
			strcat(result, insn->op_str);
		}
	}
}

