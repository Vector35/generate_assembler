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
with open('opc_examples.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^\'(.*)\': (.*)', l)
	if m:
		opc2example[m.group(1)] = int(m.group(2),16)

###############
# go!
###############

syn2example = {}

DEPTH = 3 
for opc in sorted(opc2example):
	print "// on %s" % opc
	example = opc2example[opc]

	syn = common.syntax(example)
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

			syn2 = common.syntax(example2)

			if not (syn2 == opc or syn2.startswith(opc+' ')):
				continue

			if not syn2 in syn2example:
				print "\"%s\": 0x%08X" % (syn2, example2)
				syn2example[syn2] = example2

