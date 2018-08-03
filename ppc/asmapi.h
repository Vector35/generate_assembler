
int assemble_single(string src, uint32_t addr, uint8_t *result, string& err, int *failures);

int assemble_multiline(string src, uint64_t addr, string& err);

int disasm(uint8_t *data, uint32_t addr, string& result, string& err);
