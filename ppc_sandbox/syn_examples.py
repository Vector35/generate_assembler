#!/usr/bin/env python

import re
import sys
import struct
import ctypes
import itertools

import common

###############
# read files
###############
opc2example = {}
lines = None
with open('opc_examples_nopunc.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^\'(.*)\': (.*)', l)
	if m:
		opc2example[m.group(1)] = int(m.group(2),16)

###############
# go!
###############

targets = sorted(opc2example)
if sys.argv[1:]:
	targets = [sys.argv[1]]

syn2example = {}

DEPTH = 4 
for opc in targets:
	print "// on %s" % opc
	example = opc2example[opc]

	syn = common.syntax_from_insword(example)
	if not syn in syn2example:
		print "\"%s\": 0x%08X" % (syn, example)
		syn2example[syn] = example

	# for every <DEPTH> bit positions
	for positions in itertools.combinations(range(32), DEPTH):
		for bitvalues in itertools.product([0,1], repeat=DEPTH):
			example2 = example

			for i in range(DEPTH):
				mask = 1 << positions[i]

				if bitvalues[i]:
					#print 'setting bit %d' % positions[i]
					example2 |= mask
				else:
					#print 'clearing bit %d' % positions[i]
					example2 &= ctypes.c_uint32(~mask).value

			instr2 = common.disasm(example2)
			syn2 = common.syntax_from_string(instr2)

			if not (syn2 == opc or syn2.startswith(opc+' ')):
				continue

			#print '%08X: %s %s' % (example2, instr2, syn2)

			if not syn2 in syn2example:
				print "\"%s\": 0x%08X" % (syn2, example2)
				syn2example[syn2] = example2

