#!/usr/bin/env python

import re
import sys
import struct
import ctypes
import random
import signal
import time

import common

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
START = 0
END = 0x100000000

if sys.argv[2:]:
	START = int(sys.argv[1], 16)
	END = int(sys.argv[2], 16)
	print "doing %08X to %08X" % (START, END)

t0 = time.time()

#for insword in xrange(START,END):
while 1:
	insword = random.randint(0,0xFFFFFFFF)

	syntax = common.syntax(insword)

	#if syntax in ['lwc2 CASH , ( )', 'jialc NUM ,']:
	#	print 'wtf? %08X: %s' % (insword, instr)
	#	sys.exit(-1)

	if syntax in seen:
		seen[syntax] += 1
	else:
		seen[syntax] = 1

	if status:
		t_elapsed = int(time.time() - t0);
		ips = insword / t_elapsed
		remaining = int((0x100000000 - insword) * (1.0/ips))
		print 'at %d syntaxes' % len(seen)
		#print 'insword:%08X opcs:%d t:%ds ips:%d left:%dm' % \
		#	(insword, len(seen), t_elapsed, int(ips), remaining/60)
		status = False
		signal.alarm(4)

	if qearly:
		break

for sig in sorted(seen):
	print '"%s": %d,' % (sig, seen[sig])

print '(%d syntaxes total)' % len(seen)
