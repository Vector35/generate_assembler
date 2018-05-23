/* capstone stuff */
#include <capstone/capstone.h>

int disasm(uint8_t *data, uint32_t addr_, string& distxt, string& err);

/* token stuff */
enum tok_type {
	TT_GPREG,
	TT_FREG,
	TT_WREG,
	TT_ACREG,
	TT_CASH,
	TT_NUM,
	TT_PUNC,
	TT_OPCODE,
	TT_SUFFIX,
	TT_OFFS
};

struct token {
	tok_type type;
	uint32_t ival;
	string sval;
};

int tokenize(string src, vector<token>& result, string& err);
const char* token_type_tostr(int tt);
string tokens_to_syntax(vector<token>& tokens);
string token_to_string(token t);
string tokens_to_string(vector<token>& tokens);
int enc_to_syntax(uint32_t enc, string& syntax, string& err);

