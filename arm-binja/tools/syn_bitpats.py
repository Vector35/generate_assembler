#!/usr/bin/env python

import re
import sys
import ctypes
import struct
import itertools

import common

def read_syns():
	syns = []
	seeds = []
	with open('syn_seeds.txt') as fp:
		for i,line in enumerate(fp.readlines()):
			m = re.match(r'^(........) (.*)$', line.strip())
			seeds.append(int(m.group(1),16))
			syns.append(m.group(2))
	return [syns, seeds]

def read_patterns():
	syns = []
	pats = []
	with open('syn_bitpats.txt') as fp:
		for i,line in enumerate(fp.readlines()):
			m = re.match(r'^{.*"(.*)".*$', line.strip())
			syns.append(m.group(1))
			pats.append(line.strip())
	return [syns, pats]

def pattern_generate(syn, insword):
	tmp = common.syntax_from_insword(insword)
	if syn != tmp:
		raise Exception('lookup says %08X: -%s- but we see -%s-' % (insword, syn, tmp))

	always1 = insword
	always0 = ctypes.c_uint32(~insword).value

	# fuzz the insword, see what bits are always 1, always 0
	for fuzz in common.fuzz5():
		insword2 = insword ^ fuzz

		syn2 = common.syntax_from_insword(insword2)
		if syn == syn2:
			#print '%08X: %s' % (insword2, bin(insword2)[2:])
			always1 &= insword2
			always0 &= ctypes.c_uint32(~insword2).value
		else:
			pass

	constMaskStr = ''
	for i in range(31,-1,-1):
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
	qsynq = '"%s"' % syn
	qsynq = (48-len(qsynq))*' ' + qsynq
	return "{%s,{0x%08X,0x%08X}}, // %s" % (qsynq, insword, changebits, constMaskStr)

if sys.argv[1:]:
	# this option synchronizes syn_bitpats.txt to syn_seeds.txt
	if sys.argv[1] == 'sync':
		common.disasm_libbinaryninjacore(0) # progress over noise from importing binja
		print('synchronizing...')
		[synsA,seeds] = read_syns()
		[synsB,patterns] = read_patterns()
		for (i,syn) in enumerate(synsA):
			j = synsB.index(syn) if syn in synsB else -1
			if j>=0:
				print(patterns[j])
			else:
				print(pattern_generate(syn, seeds[i]))
	# assumed a syntax is given, and that's where searching will start
	else:
		[syns,seeds] = read_syns()
		cut = syns.index(sys.argv[1])
		print('syns cut at index %d (line %d)' % (cut, cut+1))
		for i in range(cut, len(syns)):
			print(pattern_generate(syns[i], seeds[i]))
else:
	# redo everything
	[syns,seeds] = read_syns()
	print('syns cut at index %d (line %d)' % (cut, cut+1))
	for (i,syn) in enumerate(syns):
		print(pattern_generate(syn, seeds[i]))


