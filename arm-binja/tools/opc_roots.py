#!/usr/bin/env python
#
# read the raw list of possible disassemblies (opc_seeds.txt) and determine
# which opcode roots are unique (vs. having characters that are condition codes,
# or data type suffixes)
#
# by root we mean "vsubl" vs. "vsubl.s16","vsubl.s32","vsubl.s8",...

import re
import common

opcodes_full = []
lines = None
with open('opc_seeds.txt') as fp:
	for (i,line) in enumerate(fp.readlines()):
		m = re.match(r'^(.*) (.*)', line)
		if not m: raise Exception('line %d: %s malformed' % (i+1,line))
		opcodes_full.append(m.group(2))

uniques = set()

for opc_full in opcodes_full:
	[root, cc, wildcc, full, operands] = common.distxt_split(opc_full)
	uniques.add(root)

print('# %d roots' % len(uniques))
print('\n'.join(sorted(uniques)))
