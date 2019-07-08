/* a shared object that can be called easily with python+ctypes */

/* */
#include <stdint.h>
#include <string.h>

/* c++ stuff */
#include <map>
#include <string>
#include <vector>
#include <iostream>

/* binja stuff */
#include "binaryninjacore.h"
#include "binaryninjaapi.h"

BNArchitecture *arch = NULL;

extern "C" void get_disasm_binja(uint8_t *data, char *result)
{
	unsigned int i;
	size_t nBytesDisasm;
	uint8_t input[64];
	BNInstructionTextToken *ttResult = NULL;
	size_t ttCount;

	if(!arch) {
		//printf("setup start\n");
		BNSetBundledPluginDirectory("/Users/andrewl/repos/vector35/binaryninja/ui/binaryninja.app/Contents/MacOS/plugins/");
		BNInitCorePlugins();
		arch = BNGetArchitectureByName("armv7");
		//printf("setup end\n");
	}

	if(!arch) {
		printf("ERROR: BNGetArchitectureByName()\n");
		strcpy(result, "error");
		goto cleanup;
	}

	/* actually disassemble now */
	nBytesDisasm = 4;
	BNGetInstructionText(arch, (const uint8_t *)data, 0, &nBytesDisasm,
	  &ttResult, &ttCount);

	for(i=0; i<ttCount; ++i) {
		//printf("copying ttResult[%d] == \"%s\"\n", i, ttResult[i].text);
		if(i==0)
			strcpy(result, ttResult[i].text);
		else
			strcat(result, ttResult[i].text);
	}
		
	/* done! */
	cleanup:
	if(ttResult) {
		//printf("%p is ttResult that reads: %s\n", ttResult, result);
		BNFreeInstructionText(ttResult, ttCount);
	}
}

