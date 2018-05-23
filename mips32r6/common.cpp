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
		cs_mode mode = (cs_mode)(CS_MODE_LITTLE_ENDIAN | CS_MODE_MIPS32R6);
		if(cs_open(CS_ARCH_MIPS, mode, &handle) != CS_ERR_OK)
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
/* instruction tokenizing */
/*****************************************************************************/
int tokenize(string src, vector<token>& result, string& err)
{
	int rc = -1, n=0;
	char *endptr;
	const char *inbuf = src.c_str();

	map<string,int> branches = {
		{"bc1eqz",1}, {"bc1nez",1}, {"bc2eqz",1}, {"bc2nez",1},
		{"beq",1},{"beqc",1}, {"beql",1}, {"beqz",1}, {"beqzalc",1}, {"beqzc",1},
		{"beqzl",1}, {"bgec",1}, {"bgeuc",1}, {"bgez",1}, {"bgezalc",1},
		{"bgezall",1}, {"bgezc",1}, {"bgezl",1}, {"bgtz",1}, {"bgtzalc",1},
		{"bgtzc",1}, {"bgtzl",1}, {"blez",1}, {"blezalc",1}, {"blezc",1},
		{"blezl",1}, {"bltc",1}, {"bltuc",1}, {"bltz",1}, {"bltzalc",1},
		{"bltzall",1}, {"bltzc",1}, {"bltzl",1}, {"bne",1}, {"bnec",1},
		{"bnegi",1}, {"bnegi",1}, {"bnez",1}, {"bnezalc",1}, {"bnezc",1},
		{"bnezl",1}, {"bnvc",1}, {"bnz",1}, {"bz",1}, {"bz",1}, {"bz",1},
		{"bnel",1}, {"bovc",1}
	};

	result.clear();

	/* grab opcode */
	while(inbuf[n]=='_' || inbuf[n]=='.' || isalnum(inbuf[n]))
		n++;
	result.push_back({TT_OPCODE, 0, string(inbuf, n)});
	inbuf += n;

	/* loop over the rest */
	int i=0;
	while(inbuf[0]) {
		char c = inbuf[0];
		char d = inbuf[1];
		char e = inbuf[2];

		/* skip spaces */
		if(c == ' ') {
			inbuf += 1;
		}
		/* tokens starting with dollar sign */
		else if(c=='$') {
			/* freg's */
			if(d=='f' && isdigit(e)) {
				uint32_t value = strtoul(inbuf+2, &endptr, 10);
				result.push_back({TT_FREG, value, ""});
				inbuf = endptr;
			}
			/* wreg's */
			else if(d=='w') {
				uint32_t value = strtoul(inbuf+2, &endptr, 10);
				result.push_back({TT_WREG, value, ""});
				inbuf = endptr;
			}
			/* acreg's */
			else if(d=='a' && e=='c') {
				uint32_t value = strtoul(inbuf+3, &endptr, 10);
				result.push_back({TT_ACREG, value, ""});
				inbuf = endptr;
			}
			/* $zero */
			else if(d=='z' && e=='e') {
				if(strncmp(inbuf, "$zero", 5)) {
					err = "expected $zero";
					MYLOG("ERROR: %s\n", err.c_str());
					goto cleanup;
				}
				result.push_back({TT_GPREG, 0, ""});
				inbuf += 5;
			}
			/* $0 and such */
			else if(isdigit(d)) {
				uint32_t value = strtoul(inbuf+1, &endptr, 10);
				result.push_back({TT_CASH, value, ""});
				inbuf = endptr;
			}
			/* we hope it's some other register we recognize */
			else {
				map<string,uint32_t> aliases = {
					{"$at",1}, {"$v0",2}, {"$v1",3},
					{"$a0",4}, {"$a1",5}, {"$a2",6}, {"$a3",7},
					{"$t0",8}, {"$t1",9}, {"$t2",10}, {"$t3",11},
					{"$t4",12}, {"$t5",13}, {"$t6",14}, {"$t7",15}, {"$t8",24}, {"$t9",25},
					{"$s0",16}, {"$s1",17}, {"$s2",18}, {"$s3",19},
					{"$s4",20}, {"$s5",21}, {"$s6",22}, {"$s7",23}, {"$s8",30},
					{"$k0",26}, {"$k1",27}, {"$gp",28}, {"$sp",29}, {"$fp",30}, {"$ra",31}
				};

				string reg = string(inbuf, 3);
				if(aliases.find(reg) == aliases.end()) {
					err = "unrecognized alias register: " + reg;
					MYLOG("ERROR: %s\n", err.c_str());
					goto cleanup;
				}

				result.push_back({TT_GPREG, aliases[reg], ""});

				inbuf += 3;
			}
		}
		/* hexadecimal immediates */
		else if((c=='0' && d=='x') || (c=='-' && d=='0' && e=='x')) {
			uint32_t value = strtoul(inbuf, &endptr, 16);
			result.push_back({TT_NUM, value, ""});
			inbuf = endptr;
		}
		/* decimal immediates */
		else if(isdigit(c) || (c=='-' && isdigit(d))) {
			uint32_t value = strtoul(inbuf, &endptr, 10);
			result.push_back({TT_NUM, value, ""});
			inbuf = endptr;
		}
		/* punctuation */
		else if(c=='[' || c==']' || c=='(' || c==')' || c==',' || c=='.' || c=='*' || c=='+' || c=='-') {
			result.push_back({TT_PUNC, 0, string(inbuf,1)});
			inbuf += 1;
		}
		/* wtf? */
		else {
			err = "error at: " + string(inbuf) + " (original string: " + src.c_str() + ")";
			goto cleanup;
		}
	}

	/* if the opcode is a brancher, and the last token is a NUM, set it to OFFS */


	if(branches.find(result[0].sval) != branches.end())
		if(result[result.size()-1].type == TT_NUM)
			result[result.size()-1].type = TT_OFFS;

	/* done */
	rc = 0;
	cleanup:
	return rc;
}

const char* token_type_tostr(int tt)
{
	switch(tt) {
		case TT_GPREG: return "GPREG";
		case TT_FREG: return "FREG";
		case TT_WREG: return "WREG";
		case TT_ACREG: return "ACREG";
		case TT_CASH: return "CASH";
		case TT_NUM: return "NUM";
		case TT_PUNC: return "PUNC";
		case TT_OPCODE: return "OPC";
		case TT_SUFFIX: return "SUFFIX";
		case TT_OFFS: return "OFFS";
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

		switch(t.type) {
			case TT_GPREG:
			case TT_FREG:
			case TT_WREG:
			case TT_ACREG:
			case TT_CASH:
			case TT_NUM:
			case TT_OFFS:
				result += token_type_tostr(t.type);
				break;
			/* most strings are just printed out directly */
			case TT_PUNC:
			case TT_OPCODE:
			case TT_SUFFIX:
				result += t.sval;
				break;
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
		case TT_GPREG:
		case TT_FREG:
		case TT_WREG:
		case TT_ACREG:
		case TT_CASH:
		case TT_NUM:
		case TT_OFFS:
		{
			char buf[64];
			sprintf(buf, "%08X", (uint32_t)t.ival);
			result += buf;
			break;
		}
		case TT_SUFFIX:
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


