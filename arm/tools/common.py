import re
import ctypes
import struct

import itertools

def tokenize(string):
	string_ = string

	#
	# pre-tokenize
	#
	cc = ['eq','ge','gt','hi','hn','hs','le','lo','ls','lt','mi','ne','pl','vc','vs']
	digits = list('1234567890')
	hdigits = digits + list('abcdef')
	letters = list('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZ')
	alphanum = digits + letters
	punc = list('[](),.*+-!^:')

	ptoks = []

	n = 0

	# opcode
	while string[n:] and string[n] != ' ':
		n += 1
	ptoks.append(string[0:n])
	string = string[n:]

	# operands
	while string:
		# stretches of letters/nums
		if string[0] in letters:
			n = 1
			while string[n:] and string[n] in (alphanum + ['_']):
				n += 1
			ptoks.append(string[0:n])
			string = string[n:]

		# immediates
		elif string[0] == '#':
			n = 1
			while string[n:] and string[n] in (hdigits + list('-+xe.')):
				n += 1
			ptoks.append(string[0:n])
			string = string[n:]

		# hex literals
		elif string[0] == '0' and (string[1:] and string[1]=='x'):
			n = 2
			while string[n:] and string[n] in hdigits:
				n += 1
			ptoks.append(string[0:n])
			string = string[n:]

		# decimal literals, like the 5 in "vmla.i32 q0, q0, d0[5]"
		elif string[0] in digits:
			n = 1
			while string[n:] and string[n] in digits:
				n += 1
			ptoks.append(string[0:n])
			string = string[n:]

		# punctuation
		elif string[0] in punc:
			ptoks.append(string[0])
			string = string[1:]

		# register lists {...}
		elif string[0] == '{':
			n = 1
			while string[n:] and not string[n] == '}':
				n += 1
			ptoks.append(string[0:n+1])
			string = string[n+1:]

		# spaces discarded
		elif string[0] == ' ':
			string = string[1:]
		else:
			raise Exception('pretokenize stumped on: -%s- (original input: %s)' % (string, string_))

	#print 'pre tokens: ' + ' '.join(ptoks)

	#
	# real-tokenize
	#
	alias2gpr = {'sb':9,'sl':10,'fp':11,'ip':12,'sp':13,'lr':14,'pc':15}
	shifts = ['lsl','lsr','asr','rrx','ror']
	toks = [ptoks[0]]
	for tok in ptoks[1:]:
		if re.match(r'^[acs]psr_?.*', tok):
			toks.append('STATR')
		elif re.match(r'^r\d+', tok):
			toks.append('GPR')
		elif re.match(r'^q\d+', tok):
			toks.append('QREG')
		elif re.match(r'^d\d+', tok):
			toks.append('DREG')
		elif re.match(r'^p\d+', tok):
			toks.append('PREG')
		elif re.match(r'^c\d+', tok):
			toks.append('CREG')
		elif re.match(r'^s\d+', tok):
			toks.append('SREG')
		elif tok in alias2gpr:
			toks.append('GPR')
		elif re.match(r'^mvfr\d', tok):
			toks.append('MEDIAREG')
		elif tok in ['none', 'a', 'i', 'f', 'ai', 'af', 'if', 'aif']:
			toks.append('IRQ')
		elif tok in shifts:
			toks.append('SHIFT')
		elif re.match(r'^{.*?}', tok):
			toks.append('RLIST')
		elif re.match(r'^#?-?0x[a-fA-F0-9]+', tok):
			toks.append('NUM')
		elif re.match(r'^#?-?\d+', tok):
			toks.append('NUM')
		elif tok in list('[](),.*+-!^:') + ['CC','le','be']:
			toks.append(tok)
		elif tok in ['.s8','.s16','.s32','.s64','.u32','.8','.16','.32','.i32','.i64']:
			toks.append(tok)
		elif re.match(r'^[aif]+', tok):
			toks.append('OPT')
		elif tok in ['sy','st','ish','ishst','nsh','nshst','osh','oshst']: # dmb options
			toks.append('OPT')
		else:
			raise Exception('tokenize stumped on: -%s- (original input: %s)' % (tok, string_))

	# done
	return toks

gofer = None
cbuf = None
def disasm(insword):
	# initialize disassembler, if necessary
	global gofer, cbuf
	if not gofer:
		gofer = ctypes.CDLL("gofer.so")
		cbuf = ctypes.create_string_buffer(256)

	data = struct.pack('<I', insword)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))

	#print 'disassembled %08X to -%s-' % (insword, cbuf.value)
	return cbuf.value

def syntax_from_string(instr):
	tokens = tokenize(instr)
	return ' '.join(tokens)

def syntax_from_insword(insword):
	return syntax_from_string(disasm(insword))

# return all 32-bit values that have 4, 3, 2, 1 and 0 bits set
def fuzz4():
	fuzz = [0]

	for positions in itertools.combinations(range(32), 1):
		mask = (1<<positions[0])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 2):
		mask = (1<<positions[0])|(1<<positions[1])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 3):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 4):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])
		fuzz.append(mask)

	# fuzz should have all 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz

def fuzz5():
	fuzz = fuzz4()

	for positions in itertools.combinations(range(32), 5):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])|(1<<positions[4])
		fuzz.append(mask)

	# fuzz should have all 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29*28/120 + 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz

def fuzz6():
	fuzz = fuzz5()

	for positions in itertools.combinations(range(32), 6):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])|(1<<positions[4])|(1<<positions[5])
		fuzz.append(mask)

	# fuzz should have all 5-bit subets, 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29*28*27/720 + 32*31*30*29*28/120 + 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz
