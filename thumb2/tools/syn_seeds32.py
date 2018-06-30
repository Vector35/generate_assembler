#!/usr/bin/env python

import re
import sys
import struct
import ctypes
import itertools
import signal
import time
import struct

import common

###############
# read files
###############
opc2seed = {}
lines = None
with open('opc_seeds32.txt') as fp:
	lines = fp.readlines()
for l in lines:
	#print "l: -%s-" % l
	m = re.match(r'^"(.*)": "\\x(..)\\x(..)\\x(..)\\x(..)" //.*', l)
	if m:
		opc2seed[m.group(1)] = (int(m.group(2),16)<<24) | (int(m.group(3),16)<<16) | (int(m.group(4),16)<<8) | int(m.group(5),16)
	else:
		print "wtf? -%s-" % l

#print opc2seed

###############
# go!
###############

fuzz = common.fuzz5()

targets = sorted(opc2seed)

syn2seed = {}

for opc in targets:

	seed = opc2seed[opc]

	syn = common.syntax_from_string(common.disasm32(seed))
	if syn in syn2seed:
		#print 'skipping %s' % opc
		continue

	print "// on %08X: %s (initial syntax: %s)" % (seed, opc, syn)

	for (i,mask) in enumerate(fuzz):
		seed2 = seed ^ mask
		instr2 = common.disasm32(seed2)
		syn2 = common.syntax_from_string(instr2)

		#print 'syn2: %s' % syn2

		if syn2 == opc or syn2.startswith(opc+' '):
			if not (syn2 in syn2seed):
				print '"%s": 0x%08X' % (syn2, seed2)
				syn2seed[syn2] = seed2


