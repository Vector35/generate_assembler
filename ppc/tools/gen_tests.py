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
with open('syn_bitpatterns.txt') as fp:
	lines = fp.readlines()
for l in lines:
	# 'add . GPR , GPR , GPR': 0x7C000215,0x03FFF800
	m = re.search(r'\'(.*)\': (.*),(.*)', l)
	if m:
		syn2seed[m.group(1)] = int(m.group(2),16)
		syn2mask[m.group(1)] = int(m.group(3),16)

###############
# go!
###############

seen = {}
fuzz = common.fuzz6()

syntaxes = sorted(syn2seed)

for syn in syntaxes:
	shuffle(fuzz)
	seed = syn2seed[syn]
	mask = syn2mask[syn]
	distxt = common.disasm(seed)
	syn = common.syntax_from_string(distxt)

	print "\t// examples of %s" % syn

	collection = 0
	for f in fuzz:
		seed2 = seed ^ (mask & f)
		distxt2 = common.disasm(seed2)
		syn2 = common.syntax_from_string(distxt2)

		if syn != syn2:
			continue

		if distxt2 in seen:
			continue

		tmp = struct.pack('>I', seed2)
		print "\t'\\x%02X\\x%02X\\x%02X\\x%02X','%s'" % (ord(tmp[0]), ord(tmp[1]), ord(tmp[2]), ord(tmp[3]), distxt2)
		seen[distxt2] = True

		collection += 1
		if collection >= 4:
			break;

