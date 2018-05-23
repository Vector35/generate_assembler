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

fuzz = common.fuzz4()

targets = sorted(opc2seed)
if sys.argv[1:]:
	targets = [sys.argv[1]]

syn2seed = {}

for opc in targets:
	seed = opc2seed[opc]
	#print "// on %08X: %s" % (seed, opc)

	for mask in fuzz:
		seed2 = seed ^ mask
		instr2 = common.disasm(seed2)
		syn2 = common.syntax_from_string(instr2)

		if syn2 == opc or syn2.startswith(opc+' '):
			if not (syn2 in syn2seed):
				print '"%s": 0x%08X" % (syn2, seed2)
				syn2seed[syn2] = seed2


