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

	map<string,int> opcs = {
		{"adc",1}, {"add",1}, {"adr",1}, {"aesd",1}, {"aese",1}, {"aesimc",1}, {"aesmc",1}, {"and",1},
		{"bfc",1}, {"bfi",1}, {"bic",1}, {"bkpt",1}, {"bl",1}, {"blx",1}, {"bx",1}, {"bxj",1}, {"b",1}, {"cdp",1}, {"cdp2",1},
		{"clrex",1}, {"clz",1}, {"cmn",1}, {"cmp",1}, {"cps",1}, {"crc32b",1}, {"crc32cb",1}, {"crc32ch",1}, {"crc32cw",1},
		{"crc32h",1}, {"crc32w",1}, {"dbg",1}, {"dmb",1}, {"dsb",1}, {"eor",1}, {"vmov",1}, {"fldmdbx",1}, {"fldmiax",1},
		{"vmrs",1}, {"fstmdbx",1}, {"fstmiax",1}, {"hint",1}, {"hlt",1}, {"isb",1}, {"lda",1}, {"ldab",1}, {"ldaex",1},
		{"ldaexb",1}, {"ldaexd",1}, {"ldaexh",1}, {"ldah",1}, {"ldc2l",1}, {"ldc2",1}, {"ldcl",1}, {"ldc",1}, {"ldmda",1},
		{"ldmdb",1}, {"ldm",1}, {"ldmib",1}, {"ldrbt",1}, {"ldrb",1}, {"ldrd",1}, {"ldrex",1}, {"ldrexb",1}, {"ldrexd",1},
		{"ldrexh",1}, {"ldrh",1}, {"ldrht",1}, {"ldrsb",1}, {"ldrsbt",1}, {"ldrsh",1}, {"ldrsht",1}, {"ldrt",1}, {"ldr",1},
		{"mcr",1}, {"mcr2",1}, {"mcrr",1}, {"mcrr2",1}, {"mla",1}, {"mls",1}, {"mov",1}, {"movt",1}, {"movw",1}, {"mrc",1},
		{"mrc2",1}, {"mrrc",1}, {"mrrc2",1}, {"mrs",1}, {"msr",1}, {"mul",1}, {"mvn",1}, {"orr",1}, {"pkhbt",1}, {"pkhtb",1},
		{"pldw",1}, {"pld",1}, {"pli",1}, {"qadd",1}, {"qadd16",1}, {"qadd8",1}, {"qasx",1}, {"qdadd",1}, {"qdsub",1},
		{"qsax",1}, {"qsub",1}, {"qsub16",1}, {"qsub8",1}, {"rbit",1}, {"rev",1}, {"rev16",1}, {"revsh",1}, {"rfeda",1},
		{"rfedb",1}, {"rfeia",1}, {"rfeib",1}, {"rsb",1}, {"rsc",1}, {"sadd16",1}, {"sadd8",1}, {"sasx",1}, {"sbc",1},
		{"sbfx",1}, {"sdiv",1}, {"sel",1}, {"setend",1}, {"sha1c",1}, {"sha1h",1}, {"sha1m",1}, {"sha1p",1}, {"sha1su0",1},
		{"sha1su1",1}, {"sha256h",1}, {"sha256h2",1}, {"sha256su0",1}, {"sha256su1",1}, {"shadd16",1},
		{"shadd8",1}, {"shasx",1}, {"shsax",1}, {"shsub16",1}, {"shsub8",1}, {"smc",1}, {"smlabb",1}, {"smlabt",1},
		{"smlad",1}, {"smladx",1}, {"smlal",1}, {"smlalbb",1}, {"smlalbt",1}, {"smlald",1}, {"smlaldx",1},
		{"smlaltb",1}, {"smlaltt",1}, {"smlatb",1}, {"smlatt",1}, {"smlawb",1}, {"smlawt",1}, {"smlsd",1},
		{"smlsdx",1}, {"smlsld",1}, {"smlsldx",1}, {"smmla",1}, {"smmlar",1}, {"smmls",1}, {"smmlsr",1}, {"smmul",1},
		{"smmulr",1}, {"smuad",1}, {"smuadx",1}, {"smulbb",1}, {"smulbt",1}, {"smull",1}, {"smultb",1}, {"smultt",1},
		{"smulwb",1}, {"smulwt",1}, {"smusd",1}, {"smusdx",1}, {"srsda",1}, {"srsdb",1}, {"srsia",1}, {"srsib",1},
		{"ssat",1}, {"ssat16",1}, {"ssax",1}, {"ssub16",1}, {"ssub8",1}, {"stc2l",1}, {"stc2",1}, {"stcl",1}, {"stc",1},
		{"stl",1}, {"stlb",1}, {"stlex",1}, {"stlexb",1}, {"stlexd",1}, {"stlexh",1}, {"stlh",1}, {"stmda",1}, {"stmdb",1},
		{"stm",1}, {"stmib",1}, {"strbt",1}, {"strb",1}, {"strd",1}, {"strex",1}, {"strexb",1}, {"strexd",1}, {"strexh",1},
		{"strh",1}, {"strht",1}, {"strt",1}, {"str",1}, {"sub",1}, {"svc",1}, {"swp",1}, {"swpb",1}, {"sxtab",1},
		{"sxtab16",1}, {"sxtah",1}, {"sxtb",1}, {"sxtb16",1}, {"sxth",1}, {"teq",1}, {"trap",1}, {"tst",1}, {"uadd16",1},
		{"uadd8",1}, {"uasx",1}, {"ubfx",1}, {"udf",1}, {"udiv",1}, {"uhadd16",1}, {"uhadd8",1}, {"uhasx",1}, {"uhsax",1},
		{"uhsub16",1}, {"uhsub8",1}, {"umaal",1}, {"umlal",1}, {"umull",1}, {"uqadd16",1}, {"uqadd8",1}, {"uqasx",1},
		{"uqsax",1}, {"uqsub16",1}, {"uqsub8",1}, {"usad8",1}, {"usada8",1}, {"usat",1}, {"usat16",1}, {"usax",1},
		{"usub16",1}, {"usub8",1}, {"uxtab",1}, {"uxtab16",1}, {"uxtah",1}, {"uxtb",1}, {"uxtb16",1}, {"uxth",1},
		{"vabal",1}, {"vaba",1}, {"vabdl",1}, {"vabd",1}, {"vabs",1}, {"vacge",1}, {"vacgt",1}, {"vadd",1}, {"vaddhn",1},
		{"vaddl",1}, {"vaddw",1}, {"vand",1}, {"vbic",1}, {"vbif",1}, {"vbit",1}, {"vbsl",1}, {"vceq",1}, {"vcge",1},
		{"vcgt",1}, {"vcle",1}, {"vcls",1}, {"vclt",1}, {"vclz",1}, {"vcmp",1}, {"vcmpe",1}, {"vcnt",1}, {"vcvta",1},
		{"vcvtb",1}, {"vcvt",1}, {"vcvtm",1}, {"vcvtn",1}, {"vcvtp",1}, {"vcvtt",1}, {"vdiv",1}, {"vdup",1}, {"veor",1},
		{"vext",1}, {"vfma",1}, {"vfms",1}, {"vfnma",1}, {"vfnms",1}, {"vhadd",1}, {"vhsub",1}, {"vld1",1}, {"vld2",1},
		{"vld3",1}, {"vld4",1}, {"vldmdb",1}, {"vldmia",1}, {"vldr",1}, {"vmaxnm",1}, {"vmax",1}, {"vminnm",1}, {"vmin",1},
		{"vmla",1}, {"vmlal",1}, {"vmls",1}, {"vmlsl",1}, {"vmovl",1}, {"vmovn",1}, {"vmsr",1}, {"vmul",1}, {"vmull",1},
		{"vmvn",1}, {"vneg",1}, {"vnmla",1}, {"vnmls",1}, {"vnmul",1}, {"vorn",1}, {"vorr",1}, {"vpadal",1}, {"vpaddl",1},
		{"vpadd",1}, {"vpmax",1}, {"vpmin",1}, {"vqabs",1}, {"vqadd",1}, {"vqdmlal",1}, {"vqdmlsl",1}, {"vqdmulh",1},
		{"vqdmull",1}, {"vqmovun",1}, {"vqmovn",1}, {"vqneg",1}, {"vqrdmulh",1}, {"vqrshl",1}, {"vqrshrn",1},
		{"vqrshrun",1}, {"vqshl",1}, {"vqshlu",1}, {"vqshrn",1}, {"vqshrun",1}, {"vqsub",1}, {"vraddhn",1},
		{"vrecpe",1}, {"vrecps",1}, {"vrev16",1}, {"vrev32",1}, {"vrev64",1}, {"vrhadd",1}, {"vrinta",1}, {"vrintm",1},
		{"vrintn",1}, {"vrintp",1}, {"vrintr",1}, {"vrintx",1}, {"vrintz",1}, {"vrshl",1}, {"vrshrn",1}, {"vrshr",1},
		{"vrsqrte",1}, {"vrsqrts",1}, {"vrsra",1}, {"vrsubhn",1}, {"vseleq",1}, {"vselge",1}, {"vselgt",1},
		{"vselvs",1}, {"vshll",1}, {"vshl",1}, {"vshrn",1}, {"vshr",1}, {"vsli",1}, {"vsqrt",1}, {"vsra",1}, {"vsri",1},
		{"vst1",1}, {"vst2",1}, {"vst3",1}, {"vst4",1}, {"vstmdb",1}, {"vstmia",1}, {"vstr",1}, {"vsub",1}, {"vsubhn",1},
		{"vsubl",1}, {"vsubw",1}, {"vswp",1}, {"vtbl",1}, {"vtbx",1}, {"vcvtr",1}, {"vtrn",1}, {"vtst",1}, {"vuzp",1},
		{"vzip",1}, {"addw",1}, {"asr",1}, {"dcps1",1}, {"dcps2",1}, {"dcps3",1}, {"it",1}, {"lsl",1}, {"lsr",1}, {"asrs",1},
		{"lsrs",1}, {"orn",1}, {"ror",1}, {"rrx",1}, {"subs",1}, {"subw",1}, {"tbb",1}, {"tbh",1}, {"cbnz",1}, {"cbz",1},
		{"movs",1}, {"pop",1}, {"push",1}, {"nop",1}, {"yield",1}, {"wfe",1}, {"wfi",1}, {"sev",1}, {"sevl",1}, {"vpush",1},
		{"vpop",1}};
	map<string,int> ccs = {{"eq",1},{"ne",1},{"cs",1},{"hs",1},{"cc",1},{"lo",1},{"mi",1},
		{"pl",1},{"vs",1},{"vc",1},{"hi",1},{"ls",1},{"ge",1},{"lt",1},{"gt",1},{"le",1}};
	map<string,uint32_t> alias2gpr = {{"sb",9},{"sl",10},{"fp",11},{"ip",12},{"sp",13},
		{"lr",14},{"pc",15}};
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
	while(*inbuf=='.' || isalnum(*inbuf))
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
	
		if(it==pretoks.begin()) {
			int l = tok.length();

			for(int l = tok.length(); l>0; --l) {
				string cand = tok.substr(0, l);
				if
			/* must separate opcode from "cc" and shit */
			result.push_back({TT_OPCODE, 0, tok});


		}
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


