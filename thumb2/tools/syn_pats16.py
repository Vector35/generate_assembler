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
syn2seed = {}
lines = None
with open('syn_seeds16.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^"(.*)": (.*)', l)
	if m:
		syn2seed[m.group(1)] = int(m.group(2),16)

###############
# go!
###############
targets = sorted(syn2seed)
if sys.argv[1:]:
	targets = [sys.argv[1]]

for syn in targets:
	seed = syn2seed[syn]

	syn2 = common.syntax_from_insword16(seed)
	if syn != syn2:
		print 'ERROR: lookup says %08X: -%s- but we see -%s-' % \
			(seed, syn, syn2)
		sys.exit(-1)

	always1 = seed
	always0 = ctypes.c_uint16(~seed).value

	# fuzz the seed, see what bits are always 1, always 0
	for fuzz in common.fuzz6_16():
		seed2 = seed ^ fuzz

		syn2 = common.syntax_from_insword16(seed2)
		if syn == syn2:
			#print '%08X: %s' % (seed2, bin(seed2)[2:])
			always1 &= seed2
			always0 &= ctypes.c_uint16(~seed2).value
		else:
			pass

	constMaskStr = ''
	for i in xrange(15,-1,-1):
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
	changebits = ctypes.c_uint16(~constbits).value
	print "{%s,{0x%04X,0x%04X}}, // %s" % (('"'+syn+'"').rjust(32), seed, changebits, constMaskStr)

