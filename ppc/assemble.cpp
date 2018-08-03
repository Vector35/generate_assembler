#include <stdint.h>

#include <string>
using namespace std;

#include "asmapi.h"

int main(int ac, char **av)
{
	int rc = -1;

	char *line;
	string all, err;
	size_t len;

	FILE *fp = fopen(av[1], "r");
	if(!fp) {
		printf("ERROR: fopen(%s)\n", av[1]);
		goto cleanup;
	}

	while(getline(&line, &len, fp) != -1) {
		all += line;
	}

	fclose(fp);

	rc = assemble_multiline(all, 0, err);
	if(rc) {
		printf("%s\n", err.c_str());
		goto cleanup;
	}

	rc = 0;
	cleanup:
	return rc;
}
