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

extern "C" void get_disasm_capstone(uint8_t *data, int len, char *distxt, int *dtlen)
{
	int rc = -1;

	/* one-time capstone init stuff */
	static bool init = false;
	static csh handle = 0;
	static cs_insn *insn = NULL;
	if (!init) {
		cs_mode mode = (cs_mode)(CS_MODE_BIG_ENDIAN | CS_MODE_THUMB);
		if(cs_open(CS_ARCH_ARM, mode, &handle) != CS_ERR_OK) {
			printf("ERROR: cs_open()\n");
			exit(-1);
		}
		insn = cs_malloc(handle);
		init = true;
	}

	/* actually disassemble */
	uint64_t addr = 0;
	size_t len2 = len;
	const uint8_t *pinsword = data;

	if(cs_disasm_iter(handle, &pinsword, &len2, &addr, insn)) {
		/* copy out string */
		strcpy(distxt, insn->mnemonic);
		if(insn->op_str[0]) {
			strcat(distxt, " ");
			strcat(distxt, insn->op_str);
		}

		/* copy out disassembled length */
		*dtlen = len - len2;
	}

	else {
		if(cs_errno(handle) == CS_ERR_OK) {
			if(distxt)
				strcpy(distxt, "undef");
		}
		else {
			printf("ERROR: cs_disasm_iter()\n");
			exit(-1);
		}
	}

}

