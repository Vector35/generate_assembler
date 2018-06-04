#!/usr/bin/env python

import re
import sys
import struct
import ctypes
import itertools
import signal
import time

import common

###############
# read files
###############
opc2seed = {}
lines = None
with open('opc_seeds.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^"(.*)": (.*)', l)
	if m:
		opc2seed[m.group(1)] = int(m.group(2),16)

###############
# go!
###############

fuzz = common.fuzz5()

targets = sorted(opc2seed)

if sys.argv[1:] and sys.argv[1].startswith('--survey='):
	opc = sys.argv[1][9:]
	seed = opc2seed[opc]
	print "// on %08X: %s" % (seed, opc)

	for mask in fuzz:
		seed2 = seed ^ mask
		instr2 = common.disasm(seed2)

		if not instr2.startswith(opc):
			continue
				
		print instr2

	sys.exit(0)

start = targets[0]
# just an opcode, like `./syn_seeds --start=poppl`
if sys.argv[1:] and sys.argv[1].startswith('--start='):
	start = sys.argv[1][8:]

ON = False

syn2seed = {}

for opc in targets:
	if opc == start:
		ON = True

	if not ON:
		continue
		
	seed = opc2seed[opc]

	if common.syntax_from_string(common.disasm(seed)) in syn2seed:
		print 'skipping %s' % opc
		continue

	#print "// on %08X: %s" % (seed, opc)

	for (i,mask) in enumerate(fuzz):
		seed2 = seed ^ mask
		instr2 = common.disasm(seed2)
		syn2 = common.syntax_from_string(instr2)

		#print 'syn2: %s' % syn2

		if syn2 == opc or syn2.startswith(opc+' '):
			if not (syn2 in syn2seed):
				print '"%s": 0x%08X' % (syn2, seed2)
				syn2seed[syn2] = seed2


