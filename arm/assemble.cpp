
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
#define DEBUG_FITNESS 0
#define DEBUG_NATSEL 0
#define DEBUG_HOOKS 0
#define printf printf
//#define printf(...) while(0);

#include "table.h"
#include "common.h"

/*****************************************************************************/
/* genetic */
/*****************************************************************************/

int count_bits32(uint32_t x)
{
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	return ((((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24);
}

int count_bits16(uint16_t x)
{
	return count_bits32(x);
}

int count_bits64(uint64_t x)
{
	return count_bits32(x) + count_bits32(x >> 32);
}

float hamming_similar16(uint16_t a, uint16_t b)
{
	return (16-count_bits16(a ^ b)) / 16.0f;
}

float hamming_similar32(uint32_t a, uint32_t b)
{
	return (32-count_bits32(a ^ b)) / 32.0f;
}

float hamming_similar64(uint64_t a, uint64_t b)
{
	return (64-count_bits64(a ^ b)) / 64.0f;
}

float fitness(vector<token> dst, vector<token> src) {
	int n = dst.size();

	if(DEBUG_FITNESS) {
		string left = tokens_to_string(dst);
		string right = tokens_to_string(src);
		printf("   left: %s\n", left.c_str());
		printf("  right: %s\n", right.c_str());
	}

	/* same number of tokens */
	if(n != src.size())
		return 0;

	/* */
	float score = 0;
	float scorePerToken = 100.0 / n;

	/* for each token... */
	for(int i=0; i<n; ++i) {
		//printf("on token %d (types:%s and %s)\n", 
		//	i, token_type_tostr(src[i].type), token_type_tostr(dst[i].type));
		/* same type */
		if(src[i].type != dst[i].type)
			return 0;

		switch(src[i].type) {
			case TT_GPR:
			case TT_QREG:
			case TT_DREG:
			case TT_PREG:
			case TT_CREG:
			case TT_SREG:
			case TT_MEDIAREG:
			case TT_NUM:
			case TT_STATR:
			case TT_RLIST:
			{
				float hamming_similarity = hamming_similar32(src[i].ival, dst[i].ival);
				if(DEBUG_FITNESS) {
					printf("comparing %08X and %08X have hamming similarity %f\n",
						(uint32_t)src[i].ival, (uint32_t)dst[i].ival, hamming_similarity);
				}
				score += hamming_similarity * scorePerToken;
				break;
			}

			/* opcodes, suffixes, and punctuation must string match */
			case TT_SHIFT: /* "ror", "asr", "rrx", etc. */
			case TT_OPCODE:
			case TT_PUNC:
				if(src[i].sval == dst[i].sval)
					score += scorePerToken;
				break;
			default:
				printf("fitness() unrecognized token %s\n", token_to_string(src[i]).c_str());
		}
	}

	return score;
}

float score(vector<token> baseline, uint32_t newcomer, uint32_t addr)
{
	vector<token> toks_child;
	string err;
	string src;

	if(disasm((uint8_t *)&newcomer, addr, src, err))
		return -1;

	/* compare mnemonics before doing more work */
	string mnem = baseline[0].sval;
	if(src.compare(0, mnem.size(), mnem) != 0) {
		//printf("SHORTCUT!\n");
		return 0;
	}
	else {
		//printf("NOT SHORTCUT!\n");
	}
	
	/* mnemonics are the same, tokenize now... */
	if(tokenize(src, toks_child, err)) {
		printf("ERROR: %s\n", err.c_str());
		return 0;
	}
	
	return fitness(baseline, toks_child);
}

struct match {
	uint32_t src_hi, src_lo; // source bit range
	uint32_t dst_hi, dst_lo; // destination bit range
};

/*****************************************************************************/
/* assembling hooks */
/*****************************************************************************/

// force certain regions of bits to match
uint32_t enforce_bit_match(uint32_t inp, int bit, vector<match> matches)
{
	for(auto iter=matches.begin(); iter != matches.end(); ++iter) {
		struct match m = *iter;
		
		/* did we change a bit in the source region? */
		if(bit >= m.src_lo && bit <= m.src_hi) {
			/* compute masks */
			uint32_t a = 1<<m.src_hi;
			uint32_t b = 1<<m.src_lo;
			uint32_t src_mask = (a|(a-1)) ^ (b-1);
	
			a = 1<<m.dst_hi;
			b = 1<<m.dst_lo;
			uint32_t dst_mask = (a|(a-1)) ^ (b-1);
	
			/* mask and shift */
			inp = (inp & (~dst_mask));
			if(m.src_hi > m.dst_hi) {
				inp |= ((inp & src_mask) >> (m.src_hi - m.dst_hi));
			}
			else {
				inp |= ((inp & src_mask) << (m.dst_hi - m.src_hi));
			}
		}
	}

	return inp;
}

/* FIRST shot at manipulating the assembling process
	set the seed perhaps to a better value

*/
uint32_t hook_first(uint32_t seed, vector<token> toks)
{
//	if(seed != 0x7c000004 && seed != 0x7c000000)
//		return seed;
//
//	if(DEBUG_HOOKS) {
//		printf("manually assembling (seed=%08X): %s\n", seed, tokens_to_string(toks).c_str());
//		for(int i=0; i<toks.size(); ++i)
//			printf("toks[%d] is: %s\n", i, token_to_string(toks[i]).c_str());
//	}
//
//	uint32_t insword = 0;
//
//	switch(seed) {
//		case 0x7C000004: // INS
//		{
//			insword = 0x7C000004;
//			insword |= toks[3].ival << 21;							// rs
//			insword |= toks[1].ival << 16;							// rt
//			insword |= (toks[7].ival + toks[5].ival - 1) << 11;		// msb (pos+size-1)
//			insword |= toks[5].ival << 6;							// lsb (pos)
//			break;
//		}
//
//		case 0x7C000000: // EXT
//		{
//			insword = 0x7C000000;
//			insword |= toks[3].ival << 21;							// rs
//			insword |= toks[1].ival << 16;							// rt
//			insword |= (toks[7].ival - 1) << 11;					// msbd (size-1)
//			insword |= toks[5].ival << 6;							// lsb (pos)
//			break;
//		}
//	}

	return seed;
}

/* SECOND shot at manipulating the assembling process
	"help" bit flips also affect dependent fields

	for certain instructions with difficult inter-field dependencies
	inputs:
		seed: the seed value that may need the special case
		insword: instruction word
	bit: last bit changed
*/
uint32_t hook_middle(uint32_t seed, uint32_t insword, int bit)
{
//	switch(seed)
//	{
//		/* instructions need first two register fields to match */
//		case 0x58210000: // bgezc
//		case 0x18210000: // bgezalc
//		case 0x1C210000: // bltzalc
//		case 0x5C210000: // bltzc
//		{
//			vector<struct match> matches = {{25,21,20,16},{20,16,25,21}};
//			return enforce_bit_match(insword, bit, matches);
//		}
//		default:
//		return insword;
//	}
	return insword;
}

/* LAST shot at manipulating the assembling process

	fill in anything missing, like OFFS	
*/
uint32_t hook_last(uint32_t seed, uint32_t insword, vector<token> toks)
{
	token tok = toks[toks.size()-1];

	uint32_t result = insword;

	return result;
}

/*****************************************************************************/
/* main assembler function */
/*****************************************************************************/

#define N_OFFSPRING 1
#define N_BITS_FLIP 3
#define FAILURES_LIMIT 20000
int failures = 0;
int assemble_single(string src, uint32_t addr, uint8_t *result, string& err)
{
	int rc = -1;

	/* decompose instruction into tokens */
	vector<token> toks_src;
	if(tokenize(src, toks_src, err)) {
		return -1;
	}

	/* form syntax, look it up, or manually assemble it */
	string syn_src = tokens_to_syntax(toks_src);
	//printf("src:%s has syntax:%s\n", src.c_str(), syn_src.c_str());

	if(lookup.find(syn_src) == lookup.end()) {
		err = "invalid syntax (tried to look up: \"" + syn_src + "\")";
		return -1;
	}
	
	auto info = lookup[syn_src];
	if(DEBUG_NATSEL)
		printf("starting with seed: %08X\n", info.seed);
	info.seed = hook_first(info.seed, toks_src);

	uint32_t vary_mask = info.mask;

	/* start with the parent */
	uint32_t parent = info.seed;
	float init_score, top_score;
	init_score = top_score = score(toks_src, parent, addr);

	/* cache the xor masks */
	int n_flips = 0;
	int flipper_idx[32];
	uint32_t flipper[32];
	for(int i=0; i<32; ++i) {
		if(vary_mask & (1 << i)) {
			flipper_idx[n_flips] = i;
			flipper[n_flips++] = 1<<i;
		}
	}

	failures = 0;
	int failstreak = 0;

	/* vary the parent */
	int b1i=0;
	while(1) {
		/* winner? */
		if(top_score > 99.99) {
			printf("%08X wins!\n", parent);

			/* run last hook */
			parent = hook_last(info.seed, parent, toks_src);

			memcpy(result, &parent, 4);
			break;
		}

		bool overtake = false;

		for(; ; b1i = (b1i+1) % n_flips) {
			uint32_t child = parent ^ flipper[b1i];
			//printf("\tflipper[%d]=%08X, changing %08X -> %08X\n", b1i, flipper[b1i], parent, child);
			child = hook_middle(info.seed, child, flipper_idx[b1i]);

			//printf("b1i is now: %d\n", b1i);

			float s = score(toks_src, child, addr);
			if(s > top_score) {
				parent = child;
				top_score = s;
				overtake = true;
				b1i = (b1i+1) % n_flips;
				break;
			}

			if(DEBUG_NATSEL) {
				string tmp;
				disasm((uint8_t *)&child, addr, tmp, err);
				printf("%08X: %s fails to overtake, score %f\n", child, tmp.c_str(), s);		
			}
	
			failures++;
			if(failures > FAILURES_LIMIT) {
				printf("failure limit reached, not assembling!\n");
				err = "cannot assemble, valid operands?";
				goto cleanup;
			}

			failstreak++;
			//printf("--failstreak is now: %d\n", failstreak);
			if(failstreak >= n_flips) {
				/* generate a new parent that's at least as good as the seed */
				while(1) {
					parent = info.seed;
					for(int i=0; i<n_flips; ++i) {
						if(rand()%2) {
							parent ^= flipper[i];
							parent = hook_middle(info.seed, parent, flipper_idx[i]);
						}
					}

					top_score = score(toks_src, parent, addr);

					if(top_score >= init_score) {
						if(DEBUG_NATSEL)
							printf("reseed the parent to: %08X (score:%f) (vs:%f)\n", parent, top_score, init_score);
						break;
					}
					else {
						string tmp;
						disasm((uint8_t *)&parent, addr, tmp, err);
						if(DEBUG_NATSEL)
							printf("%08X: %s reseed fail %f\n", parent, tmp.c_str(), top_score);		
						failures++;
					}

					if(failures > FAILURES_LIMIT) {
						err = "cannot assemble, valid operands?";
						printf("failure limit reached, not assembling!\n");
						goto cleanup;
					}
				}
				failstreak = 0;
				break;
			}
		}

		if(overtake) {
			failstreak = 0;
			if(DEBUG_NATSEL) {
				string tmp;
				disasm((uint8_t *)&parent, addr, tmp, err);
				printf("%08X: %s overtakes with 1-bit flip (%d) after %d failures, score %f\n", parent, tmp.c_str(), b1i, failures, top_score);		
			}
		}
	}

	rc = 0;
	cleanup:
	return rc;
}

/*****************************************************************************/
/* string processing crap */
/*****************************************************************************/

int split_newlines(const string& chunk, vector<string> &lines)
{
	lines.clear();

	const char *p = chunk.c_str();

	int left=0;
	for(int i=0; i<chunk.size(); ) {
		if(p[i]=='\x0a') {
			if(left < i) {
				lines.push_back(string(p,left,i-left));
			}

			left = i = (i+1);
		}
		else if(i+1 < chunk.size() && p[i]=='\x0d' && p[i+1]=='\x0a') {
			if(left < i) {
				lines.push_back(string(p,left,i-left));
			}

			left = i = (i+2);
		}
		else {
			i += 1;
		}
	}

	return 0;
}

int trim_lines(vector<string> &lines)
{
	vector<string> filtered;

	for(int i=0; i<lines.size(); ++i) {
		const char *p = lines[i].c_str();
		int left = 0, right = lines[i].size()-1;

		while(isspace(p[left]))
			left += 1;

		while(right>=0 && isspace(p[right]))
			right -= 1;

		if(right >= left)
			filtered.push_back(string(p, left, right-left+1));
	}

	lines = filtered;

	return 0;
}

int assemble_multiline(const string& code, uint64_t addr, string& err)
{
	int rc = -1;
	vector<string> lines, fields;

	split_newlines(code, lines);
	trim_lines(lines);

	uint32_t vaddr;
	map<string,long> symbols;

	/* FIRST PASS */
	vaddr = addr;
	for(int i=0; i<lines.size(); ++i) {
		//printf("line %d: -%s-\n", i, lines[i].c_str());

		/* .org directive */
		if(fmt_match(".org\\S\\H", lines[i], fields)) {
			vaddr = strtol(fields[0].c_str(), 0, 16);
			if(vaddr & 0x3) {
				printf("ERROR: .org address is not 4-byte aligned\n");
				goto cleanup;
			}
			printf("PASS1, set vaddr to: %08X\n", vaddr);
		}
		/* .equ directive */
		else
		if(fmt_match(".equ\\S\\I\\s,\\s\\H", lines[i], fields)) {
			uint32_t value = strtol(fields[1].c_str(), 0, 16);
			symbols[fields[0]] = value;
			printf("PASS1, set symbol %s: %08X\n", fields[0].c_str(), value);
		}
		/* labels */
		else
		if(fmt_match("\\I:", lines[i], fields)) {
			symbols[fields[0]] = vaddr;	
			printf("PASS1, set label %s: %08X\n", fields[0].c_str(), vaddr);
		}
	}

	/* SECOND PASS */
	vaddr = addr;
	for(int i=0; i<lines.size(); ++i) {
		vector<string> fields;

		//printf("line %d: -%s-\n", i, lines[i].c_str());

		/* .org directive */
		if(fmt_match(".org\\S\\H", lines[i], fields)) {
			vaddr = strtol(fields[0].c_str(), 0, 16);
			printf("set vaddr to: %08X\n", vaddr);
		}
		/* .equ directive */
		else
		if(fmt_match(".equ\\S\\I\\s,\\s\\H", lines[i], fields)) {

		}
		/* labels */
		else
		if(fmt_match("\\I:", lines[i], fields)) {

		}
		/* comments */
		else
		if(fmt_match("\\s//\\X", lines[i], fields)) {
			
		}
		/* instructions */
		else {
			string err;
			uint8_t encoding[4];

			/* replace the last word (if it exists) with a label/symbol */
			string line = lines[i], token;
			int left = line.size()-1;
			while(left>=0 && isalnum(line[left]))
				left--;
			left += 1;
			token = line.substr(left, line.size()-left);
			if(fmt_match("\\I", token, fields)) {
				if(symbols.find(token) != symbols.end()) {
					char buf[16];
					long value = symbols[token];
					if(value < 0) {
						buf[0]='-';
						sprintf(buf+1, "0x%08X", (unsigned)(-1*value));
					}
					else {
						sprintf(buf, "0x%08X", (unsigned)value);
					}
					line.replace(left, line.size()-left, buf);
				}
				else {
					//printf("not found in symbol table\n");
				}
			}
			else {
				//printf("not an identifier\n");
			}

			/* now actually assemble */
			printf("assembling: %s at address 0x%08X\n", line.c_str(), vaddr);
			if(assemble_single(line, vaddr, encoding, err)) {
				printf("ERROR on line: %s (%s)\n", lines[i].c_str(), err.c_str());
				break;
			}

			printf("%08X: %02X %02X %02X %02X\n", vaddr, encoding[0], encoding[1], encoding[2], encoding[3]);
			vaddr += 4;
		}
	}

	rc = 0;
	cleanup:
	return rc;
}

/*****************************************************************************/
/* main */
/*****************************************************************************/

#define TEST_ADDR 0x0

int main(int ac, char **av)
{
	int rc = -1;
	uint32_t insWord = 0x800000A;
	vector<token> tokens;
	uint8_t encoding[4];

	/* statistics crap */
	string srcWorstTime, srcWorstFails;
	clock_t t0, t1;
	double tdelta, tavg=0, tsum=0, tworst=0;
	int tcount = 0;
	uint32_t insWordWorstTime, insWordWorstFails;
	int failsWorst = 0;

	srand(time(NULL));

	/* decide mode */
	#define MODE_FILE 0
	#define MODE_RANDOM 1
	#define MODE_SINGLE 2
	int mode = MODE_RANDOM;
	if(ac > 1) {
		struct stat st;
		stat(av[1], &st);
		if(S_ISREG(st.st_mode)) {
			printf("FILE MODE!\n");
			mode = MODE_FILE;
		}
		else if(!strcmp(av[1], "random")) {
			printf("RANDOM MODE!\n");
			mode = MODE_RANDOM;
		}
		else {
			printf("SINGLE MODE!\n");
			mode = MODE_SINGLE;
		}
	}
	else {
		printf("need args!\n");
		goto cleanup;
	}

	if(mode == MODE_FILE) {
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

		assemble_multiline(all, 0, err);

		return 0;	
	}

	if(mode == MODE_SINGLE) {
		string src, err;
		src = av[1];

		/* decompose instruction into tokens */
		vector<token> toks;

		if(tokenize(src, toks, err)) {
			printf("didn't even tokenize, error: %s\n", err.c_str());
			return -1;
		}

		string syn = tokens_to_syntax(toks);
		printf(" input: %s\n", src.c_str());
		printf("syntax: %s\n", syn.c_str());

		t0 = clock();
		if(assemble_single(src, TEST_ADDR, encoding, err)) {
			printf("ERROR: %s\n", err.c_str());
			return -1;
		}
		tdelta = (double)(clock()-t0)/CLOCKS_PER_SEC;

		printf("assemble_single() duration: %fs (%f assembles/sec)\n", tdelta, 1/tdelta);
		printf("converged after %d failures to %08X\n", failures, *(uint32_t *)encoding);

		return 0;
	}

	if(mode == MODE_RANDOM) {
		string src, err;

		while(1) {
			insWord = (rand()<<16) | rand();
	
			if(0 != disasm((uint8_t *)&insWord, TEST_ADDR, src, err)) {
				printf("ERROR: %s\n", err.c_str());
				goto cleanup;
			}

			if(src == "undef")
				continue;
	
			printf("%08X: %s\n", insWord, src.c_str());
	
			t0 = clock();
			if(assemble_single(src, TEST_ADDR, encoding, err)) {
				printf("ERROR: %s\n", err.c_str());
				printf("last instruction: %08X: '%s'\n", insWord, src.c_str());
				return -1;
			}
			tdelta = (double)(clock()-t0)/CLOCKS_PER_SEC;
			tsum += tdelta;
			tcount += 1;
			tavg = tsum/tcount;
			printf("assemble_single() duration: %fs, average: %fs (%f assembles/second)\n",
				tdelta, tavg, 1/tavg);
	
			if(tdelta > tworst) {
				insWordWorstTime = insWord;
				srcWorstTime = src;
				tworst = tdelta;
			}
	
			if(failures > failsWorst) {
				insWordWorstFails = insWord;
				failsWorst = failures;
				srcWorstFails = src;
			}
	
			printf("worst time: %f held by %08X: %s\n", tworst, insWordWorstTime, srcWorstTime.c_str());
			printf("worst fails: %d held by %08X: %s\n", failsWorst, insWordWorstFails, srcWorstFails.c_str());
		}

		return 0;
	}

	rc = 0;
	cleanup:
	return rc;
}
