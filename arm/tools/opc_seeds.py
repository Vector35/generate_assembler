#!/usr/bin/env python

import common

seen = {}

for seed in xrange(0, 0x100000000):
	distxt = common.disasm(seed)
	if distxt == 'undef':
		continue

	fqo = distxt
	if ' ' in fqo:
		fqo = fqo[0: fqo.index(' ')]

	opcnew = common.fqo_to_opcode(fqo)
	if not (opcnew in seen):
		seen[opcnew] = seed
		print "%08X %s" % (seed, opcnew)

