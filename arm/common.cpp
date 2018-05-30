/* common functions for MIPS tools
	1) calling capstone
	2) tokenizing
*/

/* */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* c++ stuff */
#include <map>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* capstone stuff */
#include <capstone/capstone.h>

#define DEBUG_DISASM 0
#define DEBUG_TOKEN 1
#define DEBUG_FITNESS 1
#define DEBUG_NATSEL 1
#define DEBUG_HOOKS 1
#define MYLOG printf
//#define MYLOG(...) while(0);

#include "common.h"

/*****************************************************************************/
/* capstone */
/*****************************************************************************/
int disasm(uint8_t *data, uint32_t addr_, string& distxt, string& err)
{
	int rc = -1;

	/* setup capstone */
	static thread_local csh handle = 0;
	static thread_local cs_insn *insn = NULL;
	if(insn == NULL) {
		cs_mode mode = (cs_mode)(CS_MODE_LITTLE_ENDIAN | CS_MODE_ARM);
		if(cs_open(CS_ARCH_ARM, mode, &handle) != CS_ERR_OK)
			err = "ERROR: cs_open()";

		insn = cs_malloc(handle);
		if(!insn)
			err = "cs_malloc()";
	}

	if(insn == NULL)
		return -1;

	/* invoke capstone */
	uint64_t addr = addr_;
	size_t size = 4;
	const uint8_t *pinsword = data;
	size_t count = cs_disasm_iter(handle, &pinsword, &size, &addr, insn);
	if(count != 1) {
		if(cs_errno(handle) == CS_ERR_OK) {
			distxt = "undef";
			rc = 0;
		}
		else {
			fprintf(stderr, "ERROR: cs_disasm_iter()\n");
		}
	}
	else {
		distxt = insn->mnemonic;
		if(insn->op_str[0]) {
			distxt += " ";
			distxt += insn->op_str;
		}
		rc = 0;
	}

	rc = 0;
	cleanup:
	return rc;
}

/*****************************************************************************/
/* string matching */
/*****************************************************************************/
// \S - one or more spaces
// \s - zero or more spaces
// \D - decimal number (captures)
// \H - hex number (captures)
// \I - identifier (captures)
// \x - exactly one character
// \X - anything

bool fmt_match(string fmt, string str, vector<string>& result)
{
	bool match = false;
	int i=0, j=0;
	int nfmt=fmt.size(), nstr=str.size();

	result.clear();
	while(1) {
		string fcode = fmt.substr(i,2);

		if(fcode=="\\S") {
			if(!isspace(str[j]))
				goto cleanup;
			while(isspace(str[j]))
				j += 1;
			i += 2;
		}
		else
		if(fcode=="\\s") {
			while(isspace(str[j]))
				j += 1;
			i += 2;
		}
		else
		if(fcode=="\\I") {
			if(!isalpha(str[j]))
				goto cleanup;
			int start = j;
			j += 1;
			while(isalnum(str[j]))
				j += 1;
			result.push_back(str.substr(start, j-start));
			i += 2;
		}
		else
		if(fcode=="\\H") {
			const char *raw = str.c_str();
			char *endptr;
			strtoul(raw + j, &endptr, 16);
			int len = endptr - (raw+j);
			if(!len) goto cleanup;
			result.push_back(str.substr(j, len));
			i += 2;
			j += len;
		}
		if(fcode=="\\D") {
			const char *raw = str.c_str();
			char *endptr;
			strtoul(raw + j, &endptr, 10);
			int len = endptr - (raw+j);
			if(!len) goto cleanup;
			result.push_back(str.substr(j, len));
			i += 2;
			j += len;
		}		
		else
		if(fcode=="\\X") {
			i += 2;
			j = nstr;
		}
		else
		if(fmt[i] == str[j]) {
			i += 1;
			j += 1;
		}
		else {
			goto cleanup;
		}

		if(i==nfmt && j==nstr) break;
	}

	match = true;
	cleanup:
	return match;
}

/*****************************************************************************/
/* instruction tokenizing */
/*****************************************************************************/
int reg2num(const char *reg)
{
	if(reg[0]=='r')
		return atoi(reg+1);
	if(reg[0]=='s' && reg[1]=='b')
		return 9;
	if(reg[0]=='s' && reg[1]=='l')
		return 10;
	if(reg[0]=='f' && reg[1]=='p')
		return 11;
	if(reg[0]=='i' && reg[1]=='p')
		return 12;
	if(reg[0]=='s' && reg[1]=='p')
		return 13;
	if(reg[0]=='l' && reg[1]=='r')
		return 14;
	if(reg[0]=='p' && reg[1]=='c')
		return 15;
	printf("ERROR: reg2num() on %s\n", reg);
	return -1;
}

int tokenize(string src, vector<token>& result, string& err)
{
	int rc = -1, i;
	char *endptr;
	const char *start, *inbuf = src.c_str();
	#define ISPUNC(x) (x=='['||x==']'||x=='('||x==')'||x==','||x=='.'||x=='*' \
		||x=='+'||x=='-'||x=='!'||x=='^'||x==':'||x=='^')

	map<string,uint32_t> alias2gpr = {{"sb",9},{"sl",10},{"fp",11},{"ip",12},
		{"sp",13},{"lr",14},{"pc",15}};
	map<string,uint32_t> shifts = {{"lsl",1},{"lsr",1},{"asr",1},{"rrx",1},
		{"ror",1}};
	map<string,uint32_t> dotsuffixes = {{".s8",1},{".s16",1},{".s32",1},{".s64",1},
		{".u32",1},{".8",1},{".16",1},{".32",1},{".i32",1},{".i64",1}};
	map<string,uint32_t> irqs = {{"none",0},{"a",1},{"i",2},{"f",4},{"ai",3},
		{"af",5},{"if",6},{"aif",7}};
	map<string,uint32_t> dmb_opts = {{"none",0},{"a",1},{"i",2},{"f",4},{"ai",3},
		{"af",5},{"if",6},{"aif",7}};

	/* pre tokens */
	vector<string> pretoks;

	/* opcode */
	start = inbuf;
	while(*inbuf=='_' || *inbuf=='.' || isalnum(*inbuf))
		inbuf++;
	pretoks.push_back(string(start, inbuf-start));

	while(*inbuf) {
		start = inbuf;

		/* stretches of letters/nums */
		if(isalpha(*inbuf))	{
			inbuf++;
			while(isalnum(*inbuf) || *inbuf=='_')
				inbuf++;
			pretoks.push_back(string(start, inbuf-start));
		}

		/* immediates */
		else if(*inbuf == '#') {
			inbuf++;
			while(isxdigit(*inbuf) || *inbuf=='-' || *inbuf=='+' ||
				*inbuf=='x' || *inbuf=='e' || *inbuf=='.') {
				inbuf++;
			}
			pretoks.push_back(string(start, inbuf-start));
		}

		/* hex literals */
		else if(inbuf[0]=='0' && inbuf[1]=='x') {
			inbuf += 2;
			while(isxdigit(*inbuf))
				inbuf++;
			pretoks.push_back(string(start, inbuf-start));
		}
		
		/* decimal literals */
		else if(isdigit(*inbuf)) {
			while(isdigit(*inbuf))
				inbuf++;
			pretoks.push_back(string(start, inbuf-start));
		}

		/* punctuation */
		else if(ISPUNC(*inbuf)) {
			inbuf++;
			pretoks.push_back(string(start, inbuf-start));
		}

		/* register lists {...} */
		else if(*inbuf == '{') {
			inbuf++;
			while(1) {
				if(!*inbuf) {
					err = "unterminated register list";
					goto cleanup;
				}
				if(*inbuf == '}')
					break;
				inbuf++;
			}
			inbuf++;
			pretoks.push_back(string(start, inbuf-start));
		}

		/* discard spaces */
		else if(*inbuf == ' ') {
			inbuf++;
		}

		/* otherwise, error */
		else {
			err = "unexpected character at: " + string(inbuf) + " (original input: " + string(src) + ")";
			goto cleanup;
		}
	}

	if(DEBUG_TOKEN) {
		printf("pretok: ");
		for(auto it=pretoks.begin(); it!=pretoks.end(); it++)
			printf("%s ", (*it).c_str());
		printf("\n");
	}

	/* loop over the rest */
	for(auto it=pretoks.begin(); it!=pretoks.end(); it++) {
		string tok = *it;
	
		if(it==pretoks.begin())
			result.push_back({TT_OPCODE, 0, tok});
		else if(ISPUNC(tok[0]) && tok.size()==1)
			result.push_back({TT_PUNC, 0, tok});
		else if((tok[0]=='a'||tok[0]=='c'||tok[0]=='s') && tok.substr(1,4)=="psr_") {
			uint32_t val = 0;
			for(int i=5; i<tok.size(); ++i) {
				if(tok[i]=='n') val |= 8;
				else if(tok[i]=='z') val |= 4;
				else if(tok[i]=='c') val |= 2;
				else if(tok[i]=='v') val |= 1;
			}
			result.push_back({TT_STATR, val, ""});
		}
		else if(tok[0]=='{') {
			const char *p = tok.c_str()+1;
			uint32_t val = 0;
			while(*p) {
				val |= (1<<reg2num(p));
				while(!(*p==',' || *p==' ' || *p=='}')) p++;
				while(*p==',' || *p==' ' || *p=='}') p++;
			}
			result.push_back({TT_RLIST, val, ""});
		}		
		else if(tok[0]=='r' && isdigit(tok[1]))
			result.push_back({TT_GPR, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='q' && isdigit(tok[1]))
			result.push_back({TT_QREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='d' && isdigit(tok[1]))
			result.push_back({TT_DREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='p' && isdigit(tok[1]))
			result.push_back({TT_PREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='c' && isdigit(tok[1]))
			result.push_back({TT_CREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='s' && isdigit(tok[1]))
			result.push_back({TT_SREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(tok[0]=='#') {
			if(tok.size()>=3 && tok[1]=='0' && tok[2]=='x')
				result.push_back({TT_NUM, (uint32_t)strtoul(tok.substr(3).c_str(),NULL,16),""});
			else
				result.push_back({TT_NUM, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		}

		else if(alias2gpr.find(tok) != alias2gpr.end())
			result.push_back({TT_GPR, alias2gpr[tok],""});
		else if(tok.substr(0,4)=="mvfr")
			result.push_back({TT_MEDIAREG, (uint32_t)strtoul(tok.substr(1).c_str(),NULL,10),""});
		else if(irqs.find(tok) != irqs.end())
			result.push_back({TT_IRQ, irqs[tok]});
		else if(shifts.find(tok) != shifts.end())
			result.push_back({TT_SHIFT, 0, tok});
		else if(tok[0]=='{') {
			uint32_t bitmask = 0;
			result.push_back({TT_RLIST, bitmask});
		}
		else {
			err = "tokenize() unrecognized token: " + tok + " (original string: " + src.c_str() + ")";
			goto cleanup;
		}
	}

	/* done */
	rc = 0;
	cleanup:
	return rc;
}

const char* token_type_tostr(int tt)
{
	switch(tt) {
		case TT_GPR: return "GPR";
		case TT_QREG: return "QREG";
		case TT_DREG: return "DREG";
		case TT_PREG: return "PREG";
		case TT_CREG: return "CREG";
		case TT_SREG: return "SREG";
		case TT_MEDIAREG: return "MEDIAREG";
		case TT_IRQ: return "IRQ";
		case TT_SHIFT: return "SHIFT";
		case TT_RLIST: return "RLIST";
		case TT_NUM: return "NUM";
		case TT_OPT: return "OPT";
		case TT_STATR: return "STATR";
		default:
			return "ERR_RESOLVING_TOKEN_TYPE";
	}
}

string tokens_to_syntax(vector<token>& tokens)
{
	string result;

	for(int i=0; i<tokens.size(); ++i) {
		if(i)
			result += " ";

		token t = tokens[i];
		//if(DEBUG_TOKEN)
		//	printf("token[%d]: %s\n", i, token_to_string(t).c_str());

		switch(t.type) {
			/* most strings are just printed out directly */
			case TT_PUNC:
			case TT_OPCODE:
				result += t.sval;
				break;
			default:
				result += token_type_tostr(t.type);
		}

	}

	return result;
}

int enc_to_syntax(uint32_t enc, string& result, string& err)
{
	int rc = -1;
	string distxt;
	vector<token> tokens;

	if(disasm((uint8_t *)&enc, 0, distxt, err))
		goto cleanup;
	if(tokenize(distxt, tokens, err))
		goto cleanup;
	result = tokens_to_syntax(tokens);

	rc = 0;
	cleanup:
	return rc;	
}

string token_to_string(token t)
{
	/* punctuation is just printed as is */
	if(t.type==TT_PUNC || t.type==TT_OPCODE)
		return t.sval;

	string result;
	result += token_type_tostr(t.type);
	result += "(";
	switch(t.type) {
		case TT_GPR:
		case TT_QREG:
		case TT_DREG:
		case TT_PREG:
		case TT_CREG:
		case TT_SREG:
		case TT_MEDIAREG:
		case TT_IRQ:
		case TT_SHIFT:
		case TT_RLIST:
		case TT_NUM:
		case TT_OPT:
		case TT_STATR:
		{
			char buf[64];
			sprintf(buf, "%X", (uint32_t)t.ival);
			result += buf;
			break;
		}
			result += t.sval;
			break;
		/* already handled, here to suppress compiler warnings */
		case TT_PUNC:
		case TT_OPCODE:
			break;
	}
	result += ")";
	return result;
}

string tokens_to_string(vector<token>& tokens)
{
	string result;

	for(int i=0; i<tokens.size(); ++i) {
		result += token_to_string(tokens[i]);
		if(i != tokens.size()-1)
			result += " ";
	}

	return result;
}


