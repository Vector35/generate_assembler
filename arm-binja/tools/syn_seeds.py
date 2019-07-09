#!/usr/bin/env python

import re
import sys
import common

###############
# read files
###############
fqo2insword = {}
lines = None
with open('opc_seeds.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^(.*) (.*)', l)
	if not m: raise Exception('malformed line %s' % l)
	fqo2insword[m.group(2)] = int(m.group(1),16)

###############
# go!
###############
targets = sorted(fqo2insword)
if sys.argv[1:]:
	print('starting search at: %s' % sys.argv[1])
	targets = targets[targets.index(sys.argv[1]):]	

syntaxes = set()
fuzz = common.fuzz5()
for fqo in targets:
	insword = fqo2insword[fqo]

	distxt = common.disasm_libbinaryninjacore(insword)
	opc = common.opcode_from_distxt(distxt, insword)
	syn = common.syntax_from_distxt(distxt, insword)

	# already encountered this syntax? skip!
	if syn in syntaxes:
		#print('skipping %s' % fqo)
		continue
	syntaxes.add(syn)

	#print("// on %08X: %s" % (insword, fqo))

	for (i,mask) in enumerate(fuzz):
		insword2 = insword ^ mask
		distxt2 = common.disasm_libbinaryninjacore(insword2)
		opc2 = common.opcode_from_distxt(distxt2, insword2)
		syn2 = common.syntax_from_distxt(distxt2, insword2)

		# same opcodes, but different syntaxes? save it!
		if opc == opc2 and not syn2 in syntaxes:
			print('%08X %s' % (insword2, syn2))
			syntaxes.add(syn2)

