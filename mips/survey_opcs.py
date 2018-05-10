#!/usr/bin/env python

# from /usr/local/include/capstone/mips.h we have:
#
# MIPS_INS_ABSQ_S = 0
# MIPS_INS_ADD = 1
# ...
# MIPS_INSN_JR_HB =~ 590

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

seen = {}
END = 0x1000000
#for instr_word in xrange(0,END):
while 1:
	instr_word = random.randint(0,0xFFFFFFFF)

	data = struct.pack('<I', instr_word)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))

	seen[cbuf.value.split()[0]] = True

	if status:
		print 'on instr_word: 0x%08X, collected %d opcodes so far' % (instr_word, len(seen))
		status = False
		signal.alarm(4)

	if qearly:
		break

print 'collected opcodes: ', '\n'.join(sorted(seen))
