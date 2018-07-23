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
with open('syn_pats32.txt') as fp:
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
fuzz = common.fuzz6()

syntaxes = sorted(syn2seed)

for syn in syntaxes:
	assert fuzz[0] == 0
	fuzz = fuzz[1:]
	shuffle(fuzz)
	fuzz = [0] + fuzz
	seed = syn2seed[syn]
	mask = syn2mask[syn]
	distxt = common.disasm32(seed)
	syn = common.syntax_from_string(distxt)

	if mode == 'text':
		print "\t\t# examples of %s" % syn

	tries = 0
	collection = 0
	for f in fuzz:
		seed2 = seed ^ (mask & f)
		distxt2 = common.disasm32(seed2)

		# fuzz may have turned into a 16-bit instruction, no worry...
		if not distxt2:
			continue

		syn2 = common.syntax_from_string(distxt2)

		if syn != syn2:
			continue

		if distxt2 in seen:
			continue

		if mode == 'text':
			print '\t\t[\'\\x%02X\\x%02X\\x%02X\\x%02X\',\'%s\'],' % \
				(seed2 >> 24, (seed2 & 0xFF0000)>>16, (seed2 & 0xFF00)>>8, seed2 & 0xFF, distxt2)
		else:
			sys.stdout.write(struct.pack('>I', seed2))

		seen[distxt2] = True

		collection += 1
		if collection >= 4:
			break;

		tries += 1
		if tries > 1000:
			break;

	if collection == 0:
		raise Exception('got no examples for %s' % syn)

