#!/usr/bin/env python

import re
import struct
import ctypes
import random
import signal

status = False
def alarm_handler(x,y):
	global status
	status = True

qearly = False
def int_handler(x,y):
	global qearly
	qearly = True

gofer = ctypes.CDLL("gofer.so")
cbuf = ctypes.create_string_buffer(256)

signal.signal(signal.SIGALRM, alarm_handler)
signal.signal(signal.SIGINT, int_handler)
signal.alarm(4)

def tokenize(string):
	gpr_aliases = {'$zero':0, '$at':1, '$v0':2, '$v1':3, \
			'$a0':4, '$a1':5, '$a2':6, '$a3':7, \
			'$t0':8, '$t1':9, '$t2':10, '$t3':11, '$t4':12, '$t5':13, '$t6':14, '$t7':15, '$t8':24, '$t9':25, \
			'$s0':16, '$s1':17, '$s2':18, '$s3':19, '$s4':20, '$s5':21, '$s6':22, '$s7':23, '$s8':30, \
			'$k0':26, '$k1':27, '$gp':28, '$sp':29, '$fp':30, '$ra':31}

	tokens = instr.split()

	string_ = string
	#print 'tokenizing: %s' % string
	result = []

	# pick off opcode
	opcode = re.match(r'^([\w\.]+)', string).group(1)
	result.append(('OPC', opcode))
	string = string[len(opcode):]

	# pick off the rest
	while string:
		eat = 0
		if string.startswith(' '):
			eat = 1

		# "f" registers $f0,$f1,...,$f31
		elif re.match(r'^\$f\d+', string):
			tmp = re.match(r'^(\$f\d+)', string).group(1) # eg: $f2
			result.append( ('FREG',int(tmp[2:])) )
			eat = len(tmp)
		# "w" registers $w0,$w1,...,$w31
		elif re.match(r'^\$w\d+', string):
			tmp = re.match(r'^(\$w\d+)', string).group(1) # eg: $w0
			result.append( ('WREG',int(tmp[2:])) )
			eat = len(tmp)
		# "ac" registers $ac0,$ac1,$ac2,$ac3
		elif re.match(r'^\$ac\d+', string):
			tmp = re.match(r'^(\$ac\d+)', string).group(1) # eg: $w0
			result.append( ('ACREG',int(tmp[3:])) )
			eat = len(tmp)
		# ----------------
		# many alias registers
		# ----------------
		# $zero
		elif re.match(r'^\$zero', string):
			result.append( ('GPREG',0) )
			eat = 5
		# aliases
		elif string[0:3] in gpr_aliases:
			result.append( ('GPREG',gpr_aliases[string[0:3]]) )
			eat = 3
		# hex numeric literals
		elif re.match(r'^-?0x[a-fA-F0-9]+', string):
			tmp = re.match(r'^(-?0x[a-fA-F0-9]+)', string).group(1)
			result.append( ('NUM', int(tmp,16)) )
			eat = len(tmp)			
		# numeric literals
		elif re.match(r'^-?\d+', string):
			tmp = re.match(r'(^-?\d+)', string).group(1)
			result.append( ('NUM', int(tmp)) )
			eat = len(tmp)
		# dollar literals
		elif re.match(r'^\$\d+', string):
			tmp = re.match(r'^(\$\d+)', string).group(1)
			result.append( ('DOLLAR_NUM', int(tmp[1:])) )
			eat = len(tmp)
		# punctuation, operators
		elif string[0] in list('[](),.*+-'):
			result.append( (string[0],string[0]) )
			eat = 1			
		else:
			raise Exception('dunno what to do with: %s (original input: %s)' % (string, string_))
		
		string = string[eat:]

	return result

seen = {}
END = 0x1000000
for instr_word in xrange(0,END):
#while 1:
#	instr_word = random.randint(0,0xFFFFFFFF)

	data = struct.pack('<I', instr_word)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))

	instr = cbuf.value
	tokens = tokenize(instr)
	syntax = tokens[0][1] + ' ' + ' '.join(map(lambda x: x[0], tokens[1:]))

	if syntax in seen:
		seen[syntax] += 1
	else:
		seen[syntax] = 1

	if status:
		print 'on instr_word: 0x%08X, collected %d syntaxes so far' % (instr_word, len(seen))
		status = False
		signal.alarm(4)

	if qearly:
		break

for sig in sorted(seen):
	print '"%s": %d,' % (sig, seen[sig])

print '(%d signatures total)' % len(seen)
