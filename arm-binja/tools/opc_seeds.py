#!/usr/bin/env python

import common

seen = {}

for seed in range(0, 0x100000000):
	distxt = common.disasm_libbinaryninjacore(seed).decode('utf-8')
	if distxt == 'undef':
		continue

	fqo = distxt
	if ' ' in fqo:
		fqo = fqo[0: fqo.index(' ')]

	if not (fqo in seen):
		seen[fqo] = 1
		print("%08X: %s" % (seed, distxt))


