#!/usr/bin/env python

import re
import sys
import struct
import ctypes
import itertools
import signal
import time
import struct
from random import shuffle

import common

###############
# read files
###############
syn2seed = {}
syn2mask = {}
lines = None
with open('syn_pats16.txt') as fp:
	lines = fp.readlines()
for l in lines:
	# {                "adcs GPR , GPR",{0x4140,0x003F}}, // 0100000101xxxxxx
	m = re.search(r'"(.*)",{(.*),(.*)}}', l)
	if m:
		syn2seed[m.group(1)] = int(m.group(2),16)
		syn2mask[m.group(1)] = int(m.group(3),16)

###############
# go!
###############

mode = 'text'
if sys.argv[1:] and sys.argv[1]=='bin':
	mode = 'bin'

seen = {}
fuzz = common.fuzz6_16()

syntaxes = sorted(syn2seed)

for syn in syntaxes:
	assert fuzz[0] == 0
	fuzz = fuzz[1:]
	shuffle(fuzz)
	fuzz = [0] + fuzz
	seed = syn2seed[syn]
	mask = syn2mask[syn]
	distxt = common.disasm16(seed)
	if not distxt:
		print 'shit, %08X: %s did not disassemble' % (seed, syn)
	syn = common.syntax_from_string(distxt)

	if mode == 'text':
		print "\t\t# examples of %s" % syn

	collection = 0
	for f in fuzz:
		seed2 = seed ^ (mask & f)
		distxt2 = common.disasm16(seed2)
		syn2 = common.syntax_from_string(distxt2)

		if syn != syn2:
			continue

		if distxt2 in seen:
			continue

		if mode == 'text':
			print '\t\t[\'\\x%02X\\x%02X\',\'%s\'],' % (seed2 >> 8, seed2 & 0xFF, distxt2)
		else:
			sys.stdout.write(struct.pack('>H', seed2))
		seen[distxt2] = True

		collection += 1
		if collection >= 4:
			break;

