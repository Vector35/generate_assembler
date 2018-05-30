/* capstone stuff */
#include <capstone/capstone.h>

int disasm(uint8_t *data, uint32_t addr_, string& distxt, string& err);

/* token stuff */
enum tok_type {
	TT_OPCODE,
	TT_GPR,
	TT_QREG,
	TT_DREG,
	TT_PREG,
	TT_CREG,
	TT_SREG,
	TT_MEDIAREG,
	TT_IRQ,
	TT_SHIFT,
	TT_RLIST,
	TT_NUM,
	TT_OPT,
	TT_STATR,
	TT_PUNC,
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

bool fmt_match(string fmt, string str, vector<string>& result);
