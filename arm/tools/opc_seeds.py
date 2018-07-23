#!/usr/bin/env python

import re
import sys
import common

cs_opcodes = ['adc', 'add', 'adr', 'aesd', 'aese', 'aesimc', 'aesmc', 'and',
'bfc', 'bfi', 'bic', 'bkpt', 'bl', 'blx', 'bx', 'bxj', 'b', 'cdp', 'cdp2',
'clrex', 'clz', 'cmn', 'cmp', 'cps', 'crc32b', 'crc32cb', 'crc32ch', 'crc32cw',
'crc32h', 'crc32w', 'dbg', 'dmb', 'dsb', 'eor', 'vmov', 'fldmdbx', 'fldmiax',
'vmrs', 'fstmdbx', 'fstmiax', 'hint', 'hlt', 'isb', 'lda', 'ldab', 'ldaex',
'ldaexb', 'ldaexd', 'ldaexh', 'ldah', 'ldc2l', 'ldc2', 'ldcl', 'ldc', 'ldmda',
'ldmdb', 'ldm', 'ldmib', 'ldrbt', 'ldrb', 'ldrd', 'ldrex', 'ldrexb', 'ldrexd',
'ldrexh', 'ldrh', 'ldrht', 'ldrsb', 'ldrsbt', 'ldrsh', 'ldrsht', 'ldrt', 'ldr',
'mcr', 'mcr2', 'mcrr', 'mcrr2', 'mla', 'mls', 'mov', 'movt', 'movw', 'mrc',
'mrc2', 'mrrc', 'mrrc2', 'mrs', 'msr', 'mul', 'mvn', 'orr', 'pkhbt', 'pkhtb',
'pldw', 'pld', 'pli', 'qadd', 'qadd16', 'qadd8', 'qasx', 'qdadd', 'qdsub',
'qsax', 'qsub', 'qsub16', 'qsub8', 'rbit', 'rev', 'rev16', 'revsh', 'rfeda',
'rfedb', 'rfeia', 'rfeib', 'rsb', 'rsc', 'sadd16', 'sadd8', 'sasx', 'sbc',
'sbfx', 'sdiv', 'sel', 'setend', 'sha1c', 'sha1h', 'sha1m', 'sha1p', 'sha1su0',
'sha1su1', 'sha256h', 'sha256h2', 'sha256su0', 'sha256su1', 'shadd16',
'shadd8', 'shasx', 'shsax', 'shsub16', 'shsub8', 'smc', 'smlabb', 'smlabt',
'smlad', 'smladx', 'smlal', 'smlalbb', 'smlalbt', 'smlald', 'smlaldx',
'smlaltb', 'smlaltt', 'smlatb', 'smlatt', 'smlawb', 'smlawt', 'smlsd',
'smlsdx', 'smlsld', 'smlsldx', 'smmla', 'smmlar', 'smmls', 'smmlsr', 'smmul',
'smmulr', 'smuad', 'smuadx', 'smulbb', 'smulbt', 'smull', 'smultb', 'smultt',
'smulwb', 'smulwt', 'smusd', 'smusdx', 'srsda', 'srsdb', 'srsia', 'srsib',
'ssat', 'ssat16', 'ssax', 'ssub16', 'ssub8', 'stc2l', 'stc2', 'stcl', 'stc',
'stl', 'stlb', 'stlex', 'stlexb', 'stlexd', 'stlexh', 'stlh', 'stmda', 'stmdb',
'stm', 'stmib', 'strbt', 'strb', 'strd', 'strex', 'strexb', 'strexd', 'strexh',
'strh', 'strht', 'strt', 'str', 'sub', 'svc', 'swp', 'swpb', 'sxtab',
'sxtab16', 'sxtah', 'sxtb', 'sxtb16', 'sxth', 'teq', 'trap', 'tst', 'uadd16',
'uadd8', 'uasx', 'ubfx', 'udf', 'udiv', 'uhadd16', 'uhadd8', 'uhasx', 'uhsax',
'uhsub16', 'uhsub8', 'umaal', 'umlal', 'umull', 'uqadd16', 'uqadd8', 'uqasx',
'uqsax', 'uqsub16', 'uqsub8', 'usad8', 'usada8', 'usat', 'usat16', 'usax',
'usub16', 'usub8', 'uxtab', 'uxtab16', 'uxtah', 'uxtb', 'uxtb16', 'uxth',
'vabal', 'vaba', 'vabdl', 'vabd', 'vabs', 'vacge', 'vacgt', 'vadd', 'vaddhn',
'vaddl', 'vaddw', 'vand', 'vbic', 'vbif', 'vbit', 'vbsl', 'vceq', 'vcge',
'vcgt', 'vcle', 'vcls', 'vclt', 'vclz', 'vcmp', 'vcmpe', 'vcnt', 'vcvta',
'vcvtb', 'vcvt', 'vcvtm', 'vcvtn', 'vcvtp', 'vcvtt', 'vdiv', 'vdup', 'veor',
'vext', 'vfma', 'vfms', 'vfnma', 'vfnms', 'vhadd', 'vhsub', 'vld1', 'vld2',
'vld3', 'vld4', 'vldmdb', 'vldmia', 'vldr', 'vmaxnm', 'vmax', 'vminnm', 'vmin',
'vmla', 'vmlal', 'vmls', 'vmlsl', 'vmovl', 'vmovn', 'vmsr', 'vmul', 'vmull',
'vmvn', 'vneg', 'vnmla', 'vnmls', 'vnmul', 'vorn', 'vorr', 'vpadal', 'vpaddl',
'vpadd', 'vpmax', 'vpmin', 'vqabs', 'vqadd', 'vqdmlal', 'vqdmlsl', 'vqdmulh',
'vqdmull', 'vqmovun', 'vqmovn', 'vqneg', 'vqrdmulh', 'vqrshl', 'vqrshrn',
'vqrshrun', 'vqshl', 'vqshlu', 'vqshrn', 'vqshrun', 'vqsub', 'vraddhn',
'vrecpe', 'vrecps', 'vrev16', 'vrev32', 'vrev64', 'vrhadd', 'vrinta', 'vrintm',
'vrintn', 'vrintp', 'vrintr', 'vrintx', 'vrintz', 'vrshl', 'vrshrn', 'vrshr',
'vrsqrte', 'vrsqrts', 'vrsra', 'vrsubhn', 'vseleq', 'vselge', 'vselgt',
'vselvs', 'vshll', 'vshl', 'vshrn', 'vshr', 'vsli', 'vsqrt', 'vsra', 'vsri',
'vst1', 'vst2', 'vst3', 'vst4', 'vstmdb', 'vstmia', 'vstr', 'vsub', 'vsubhn',
'vsubl', 'vsubw', 'vswp', 'vtbl', 'vtbx', 'vcvtr', 'vtrn', 'vtst', 'vuzp',
'vzip', 'addw', 'asr', 'dcps1', 'dcps2', 'dcps3', 'it', 'lsl', 'lsr', 'asrs',
'lsrs', 'orn', 'ror', 'rrx', 'subs', 'subw', 'tbb', 'tbh', 'cbnz', 'cbz',
'movs', 'pop', 'push', 'nop', 'yield', 'wfe', 'wfi', 'sev', 'sevl', 'vpush',
'vpop']

re_s = r'(?P<s>s)?'														# set flags
re_cc = r'(?P<cc>(eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le))?'	# condition code
re_q = r'(?P<q>(\.w|\.n))?'												# qualifier (wide or narrow)
re_ds1 = r'(?P<ds1>(\.\w?\d+))?'										# data size
re_ds2 = r'(?P<ds2>(\.\w?\d+))?'										# data size
regex = re_s + re_cc + re_ds1 + re_ds2 + r'$'

seen = {}

for seed in xrange(0, 0x100000000):
	distxt = common.disasm(seed)
	if distxt == 'undef':
		continue

	opcfull = distxt
	if ' ' in opcfull:
		opcfull = opcfull[0: opcfull.index(' ')]

	#print '%s (0x%X)' % (opcfull, seed)
	#print '-----------------------'

	# collect all possible opcodes this could be
	# eg: "ble" could be 'b' or 'bl'
	opcs = []
	tmp = opcfull
	while tmp:
		if (tmp in cs_opcodes):
			opcs.append(tmp)
		tmp = tmp[:-1]

	if not opcs:
		raise Exception('opcode lookup error')

	# find largest opcode where suffix makes sense
	[opc, suffix] = ['', None]
	for candidate in sorted(opcs, key=len, reverse=True):
		suffix = opcfull[len(candidate):]

		ok = False
		if candidate == 'cps' and (suffix in ['id', 'ie']):
			suffix = ''
			ok = True
		if re.match(regex, suffix):
			ok = True

		if ok:
			opc = candidate
			break
	
	if not opc:
		raise Exception('couldnt narrow candidate')

	#
	#print 'opcode: ', opc
	#print 'suffix: ', suffix

	gd = re.match(regex, suffix).groupdict()

	if 0:
		if gd['cc']:
			print '    cc: ', gd['cc']
		elif (seed >> 28) == 0xe:
			print '    cc: ', 'al'
		print '     s: ', gd['s']
		print '   ds1: ', gd['ds1']
		print '   ds2: ', gd['ds2']
		print ''
	
	opcnew = opc
	if gd['s']: opcnew += 's'
	if gd['cc']:
		opcnew += '<c>'
	elif (seed >> 28) == 0xe:
		opcnew += '<c>'
	if gd['ds1']: opcnew += gd['ds1']
	if gd['ds2']: opcnew += gd['ds2']
	#print opcnew
	#print ''

	if not (opcnew in seen):
		seen[opcnew] = seed
		print "%08X %s" % (seed, opcnew)

