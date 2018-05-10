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
	aliases = ['$zero':0, '$at':1, '$v0':2, '$v1':3, \
			'$a0':4, '$a1':5, '$a2':6, '$a3':7, \
			'$t0':8, '$t1':9, '$t2', '$t3', '$t4', '$t5', '$t6', '$t7', '$t8', '$t9' \
			'$s0', '$s1', '$s2', '$s3', '$s4', '$s5', '$s6', '$s7', \
			'$k0', '$k1', '$gp', '$sp', '$fp', '$ra']:

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

		# "f" registers
		elif re.match(r'^\$f\d+', string):
			tmp = re.match(r'^(\$f\d+)', string).group(1) # eg: $f2
			result.append( ('FREG',int(tmp[2:])) )
			eat = len(tmp)
		# "t" registers
		elif re.match(r'^\$t\d+', string):
			tmp = re.match(r'^(\$t\d+)', string).group(1) # eg: $t0
			result.append( ('TREG',int(tmp[2:])) )
			eat = len(tmp)
		# "s" registers
		elif re.match(r'^\$s\d+', string):
			tmp = re.match(r'^(\$s\d+)', string).group(1) # eg: $s0
			result.append( ('SREG',int(tmp[2:])) )
			eat = len(tmp)
		# "w" registers
		elif re.match(r'^\$w\d+', string):
			tmp = re.match(r'^(\$w\d+)', string).group(1) # eg: $w0
			result.append( ('WREG',int(tmp[2:])) )
			eat = len(tmp)
		# "ac" registers
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
		# ugh
		elif string[0:3] in ['$at', '$v0', '$v1', '$a0', '$a1', '$a2', '$a3', \
			'$t0', '$t1', '$t2', '$t3', '$t4', '$t5', '$t6', '$t7', '$t8', '$t9' \
			'$s0', '$s1', '$s2', '$s3', '$s4', '$s5', '$s6', '$s7', \
			'$k0', '$k1', '$gp', '$sp', '$fp', '$ra']:


			result.append( ('GPREG',1) )
			eat = 3
		elif string[0:3] in ['$v0', '$v1']:
			result.append( ('GPREG',2+int(string[2])) )
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
		# punctuation, operators
		elif string[0] in list('(),.*+-'):
			result.append( ('PUNC',string[0]) )
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
	print tokens

	if status:
		print 'on instr_word: 0x%08X, collected %d opcodes so far' % (instr_word, len(seen))
		status = False
		signal.alarm(4)

	if qearly:
		break

print 'collected opcodes: ', '\n'.join(sorted(seen))
