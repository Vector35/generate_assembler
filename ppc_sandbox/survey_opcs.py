#!/usr/bin/env python

import sys
import struct
import ctypes
import random
import signal
import time

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

t0 = time.time()

seen = {}
START = 0
END = 0x100000000
LIVE = False;

for k in sys.argv[1:]:
	if k == '--live':
		LIVE = True
	elif k.startswith('--start='):
		START = int(k[8:], 16)

print "is LIVE? %d" % LIVE
print "range: [%08X, %08X]" % (START, END)

for insword in xrange(START,END):
#while 1:
#	insword = random.randint(0,0xFFFFFFFF)

	data = struct.pack('<I', insword)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))

	if LIVE:
		print '%08X %s' % (insword, cbuf.value)

	seen[cbuf.value.split()[0]] = True

	if status:
		t_elapsed = int(time.time() - t0);
		ips = insword / t_elapsed
		remaining = int((0x100000000 - insword) * (1.0/ips))
		print 'insword:%08X opcs:%d t:%ds ips:%d left:%dm' % \
			(insword, len(seen), t_elapsed, int(ips), remaining/60)
		status = False
		signal.alarm(4)

	if qearly:
		break

print 'collected opcodes: ', '\n'.join(sorted(seen))
