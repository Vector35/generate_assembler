import re
import ctypes
import struct

import itertools

def tokenize(string):
	gpr_aliases = {'$zero':0, '$at':1, '$v0':2, '$v1':3, \
			'$a0':4, '$a1':5, '$a2':6, '$a3':7, \
			'$t0':8, '$t1':9, '$t2':10, '$t3':11, '$t4':12, '$t5':13, '$t6':14, '$t7':15, '$t8':24, '$t9':25, \
			'$s0':16, '$s1':17, '$s2':18, '$s3':19, '$s4':20, '$s5':21, '$s6':22, '$s7':23, '$s8':30, \
			'$k0':26, '$k1':27, '$gp':28, '$sp':29, '$fp':30, '$ra':31}

	tokens = string.split()

	string_ = string
	#print 'tokenizing: %s' % string
	result = []

	# pick off opcode
	# (includes the .fmt)
	opcode = re.match(r'^([\.\w]+)', string).group(1)
	result.append( ['OPC', opcode] )
	string = string[len(opcode):]

	# pick off the rest
	while string:
		eat = 0
		if string.startswith(' '):
			eat = 1

		# "f" registers $f0,$f1,...,$f31
		elif re.match(r'^\$f\d+', string):
			tmp = re.match(r'^(\$f\d+)', string).group(1) # eg: $f2
			result.append( ['FREG',int(tmp[2:])] )
			eat = len(tmp)
		# "w" registers $w0,$w1,...,$w31
		elif re.match(r'^\$w\d+', string):
			tmp = re.match(r'^(\$w\d+)', string).group(1) # eg: $w0
			result.append( ['WREG',int(tmp[2:])] )
			eat = len(tmp)
		# "ac" registers $ac0,$ac1,$ac2,$ac3
		elif re.match(r'^\$ac\d+', string):
			tmp = re.match(r'^(\$ac\d+)', string).group(1) # eg: $w0
			result.append( ['ACREG',int(tmp[3:])] )
			eat = len(tmp)
		# ----------------
		# many alias registers
		# ----------------
		# $zero
		elif re.match(r'^\$zero', string):
			result.append( ['GPREG',0] )
			eat = 5
		# aliases
		elif string[0:3] in gpr_aliases:
			result.append( ['GPREG',gpr_aliases[string[0:3]]] )
			eat = 3
		# hex numeric literals
		elif re.match(r'^-?0x[a-fA-F0-9]+', string):
			tmp = re.match(r'^(-?0x[a-fA-F0-9]+)', string).group(1)
			result.append( ['NUM', int(tmp,16)] )
			eat = len(tmp)			
		# numeric literals
		elif re.match(r'^-?\d+', string):
			tmp = re.match(r'(^-?\d+)', string).group(1)
			result.append( ['NUM', int(tmp)] )
			eat = len(tmp)
		# dollar literals
		elif re.match(r'^\$\d+', string):
			tmp = re.match(r'^(\$\d+)', string).group(1)
			result.append( ['CASH', int(tmp[1:])] )
			eat = len(tmp)
		# punctuation, operators
		elif string[0] in list('[](),.*+-'):
			result.append( [string[0],string[0]] )
			eat = 1			
		else:
			raise Exception('dunno what to do with: %s (original input: %s)' % (string, string_))
		
		string = string[eat:]

	# if the opcode is a branch, and the last token type is num, change it to type offset
	branch_instrs = \
		['bc1eqz', 'bc1nez', 'bc2eqz', 'bc2nez', 'beq', 'beqc', 'beql', 'begz', \
		'beqzalc', 'beqzc', 'beqzl', 'bgec', 'bgeuc', 'bgez', 'bgezalc', \
		'bgezall', 'bgezc', 'bgezl', 'bgtz', 'bgtzalc', 'bgtzc', 'bgtzl', \
		'blez', 'blezalc', 'blezc', 'blezl', 'bltc', 'bltuc', 'bltz', \
		'bltzalc', 'bltzall', 'bltzc', 'bltzl', 'bne', 'bnec', 'bnegi', \
		'bnez', 'bnezalc', 'bnezc', 'bnezl', \
		'bnvc', 'bnz', 'bz', 'bnel', 'bovc']

	if (result[0][1] in branch_instrs) and result[-1][0]=='NUM':
		result[-1][0] = 'OFFS'

	# done
	return result

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

	return cbuf.value
	
def syntax_from_string(instr):
	tokens = tokenize(instr)
	
	# first part of result is opcode
	syntax = tokens[0][1];
	tokens = tokens[1:]

	# possibly second part is suffix
	if tokens and tokens[0][0] == 'SUFFIX':
		syntax += ' '+tokens[0][1]
		tokens = tokens[1:]

	# the remainder
	if tokens:
		syntax += ' ' + ' '.join(map(lambda x: x[0], tokens))

	return syntax

def syntax_from_insword(insword):
	return syntax_from_string(disasm(insword))

# return all 32-bit values have 4, 3, 2, 1 and 0 bits set
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
