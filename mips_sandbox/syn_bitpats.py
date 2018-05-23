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
with open('syn_seeds.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^"(.*)": (.*)', l)
	if m:
		syn2seed[m.group(1)] = int(m.group(2),16)

###############
# go!
###############

fuzz = common.fuzz4()

targets = sorted(syn2seed)
if sys.argv[1:]:
	targets = [sys.argv[1]]

for syn in targets:
	seed = syn2seed[syn]

	syn2 = common.syntax_from_insword(seed)
	if syn != syn2:
		print 'ERROR: lookup says %08X: -%s- but we see -%s-' % \
			(seed, syn, syn2)
		sys.exit(-1)

	always1 = seed
	always0 = ctypes.c_uint32(~seed).value

	# for every <DEPTH> bit positions
	for mask in fuzz:
		seed2 = seed ^ mask
		syn2 = common.syntax_from_insword(seed2)
		if syn == syn2:
			always1 &= seed2
			always0 &= ctypes.c_uint32(~seed2).value
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
	print "{%s,{0x%08X,0x%08X}}, // %s %s" % \
		(('"%s"' % syn).rjust(32), seed, changebits, constMaskStr, common.disasm(seed))

