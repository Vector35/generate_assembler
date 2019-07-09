#!/usr/bin/env python
#
# read the raw list of possible disassemblies (opc_seeds.txt) and determine
# which opcodes are unique (vs. having characters that are condition codes,
# or data type suffixes)

import re
import common

ccs = ["eq", "ne", "cs", "hs", "cc", "lo", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le"]

opc2seed = {}
lines = None
with open('opc_seeds.txt') as fp:
	lines = fp.readlines()
for l in lines:
	m = re.match(r'^(.*) (.*)', l)
	if m:
		opc2seed[m.group(2)] = int(m.group(1),16)

regex = r'^' + common.re_root + common.re_s + common.re_cc + common.re_ds1 + common.re_ds2 + r'$'

opcodes = set()
for fqo in opc2seed.keys():
	m = re.match(regex,fqo)
	if not m: raise Exception('no matchy regex on: %s' % unfilt)

	nocond = m['root'] + (m['ds1'] or '') + (m['ds2'] or '')
	yescond = m['root'] + (m['cc'] or '')
	# if we identified the CC, then the opcode without CC is also present
	if nocond in opc2seed:
		print('%s has nocond %s and is in opc2seed (thus the root %s is an opcode)' % (fqo, nocond, m['root']))
		opcodes.add(m['root'])
	if not nocond in opc2seed:
		print('%s has nocond %s and is NOT opc2seed (thus yescond %s is an opcode)' % (fqo, nocond, yescond))
		opcodes.add(yescond)
		#print('%s gets mis-split into %s' % (fqo, nocond))
		pass

print('# %d opcodes' % len(opcodes))
print('opcodes = {'+
	(','.join(
		map(lambda x: ("'%s'"%x),
			sorted(opcodes)
		)
	)+
	'}'
))
