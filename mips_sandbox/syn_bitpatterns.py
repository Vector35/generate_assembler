#!/usr/bin/env python

import re
import sys
import ctypes
import struct
import itertools

import common

###############
# read files
###############
syn2example = {}
lines = None
with open('syn_examples.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^\'(.*)\': (.*)', l)
	if m:
		syn2example[m.group(1)] = int(m.group(2),16)

###############
# go!
###############
DEPTH = 4 

targets = sorted(syn2example)
if sys.argv[1:]:
	targets = [sys.argv[1]]

for syn in targets:
	example = syn2example[syn]

	syn2 = common.syntax(example)
	if syn != syn2:
		print 'ERROR: lookup says %08X: -%s- but we see -%s-' % \
			(example, syn, syn2)
		sys.exit(-1)

	always1 = example
	always0 = ctypes.c_uint32(~example).value

	# for every <DEPTH> bit positions
	for positions in itertools.combinations(range(32), DEPTH):
		for bitvalues in itertools.product([0,1], repeat=DEPTH):
			example_ = example

			for i in range(DEPTH):
				mask = 1 << positions[i]

				if bitvalues[i]:
					#print 'setting bit %d' % positions[i]
					example_ |= mask
				else:
					#print 'clearing bit %d' % positions[i]
					example_ &= ctypes.c_uint32(~mask).value

			#print 'new example: %08X %s' % (example_, bin(example_))

			syn2 = common.syntax(example_)
			if syn == syn2:
				always1 &= example_
				always0 &= ctypes.c_uint32(~example_).value
			else:
				pass

	constMaskStr = ''
	for i in xrange(31,-1,-1):
		assert not ((always1 & (1<<i)) and (always0 & (1<<i)))
		if always1 & (1<<i):
			constMaskStr += '1'
		elif always0 & (1<<i):
			constMaskStr += '0'
		else:
			constMaskStr += 'x'

	#print 'always1: %08X' % always1
	#print 'always0: %08X' % always0
	constbits = always1 | always0
	changebits = ctypes.c_uint32(~constbits).value
	print "{'%s',{0x%08X,0x%08X}}, // %s" % (syn, example, changebits, constMaskStr)

