import re
import ctypes
import struct

import itertools

# fully-qualified opcode like:  "fstmdbxge"
# to parameterized opcode like: "fstmdbx<c>"
def fqo_to_opcode(fqo, insword):
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
	
	# collect possible opcodes this could be
	opcs = []
	tmp = fqo
	while tmp:
		if (tmp in cs_opcodes):
			opcs.append(tmp)
		tmp = tmp[:-1]

	if not opcs:
		raise Exception('opcode lookup error (original input: %s)' % fqo)

	# find largest opcode where suffix makes sense
	[opc, suffix] = ['', None]
	for candidate in sorted(opcs, key=len, reverse=True):
		suffix = fqo[len(candidate):]

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

	# append the important suffix parts
	gd = re.match(regex, suffix).groupdict()
	opcnew = opc
	if gd['s']: opcnew += 's'
	if gd['cc']:
		opcnew += '<c>'
	elif (insword >> 28) == 0xe:
		opcnew += '<c>'
	if gd['ds1']: opcnew += gd['ds1']
	if gd['ds2']: opcnew += gd['ds2']
	return opcnew

def tokenize(distxt, insword):
	orig = distxt

	# opcode, operands
	[opcode, operands] = ['', '']
	if ' ' in distxt:
		fqo = distxt[0: distxt.index(' ')]
		opcode = fqo_to_opcode(fqo, insword)
		operands = distxt[distxt.index(' ')+1:]
	else:
		opcode = fqo_to_opcode(distxt, insword)
		operands = ''

	# pre-tokenize
	digits = list('1234567890')
	hdigits = digits + list('abcdef')
	letters = list('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZ')
	alphanum = digits + letters
	punc = list('[](),.*+-!^:')
	ptoks = []
	n = 0

	# operands
	while operands:
		# stretches of letters/nums
		if operands[0] in letters:
			n = 1
			while operands[n:] and operands[n] in (alphanum + ['_']):
				n += 1
			ptoks.append(operands[0:n])
			operands = operands[n:]

		# immediates
		elif operands[0] == '#':
			n = 1
			while operands[n:] and operands[n] in (hdigits + list('-+xe.')):
				n += 1
			ptoks.append(operands[0:n])
			operands = operands[n:]

		# hex literals
		elif operands[0] == '0' and (operands[1:] and operands[1]=='x'):
			n = 2
			while operands[n:] and operands[n] in hdigits:
				n += 1
			ptoks.append(operands[0:n])
			operands = operands[n:]

		# decimal literals, like the 5 in "vmla.i32 q0, q0, d0[5]"
		elif operands[0] in digits:
			n = 1
			while operands[n:] and operands[n] in digits:
				n += 1
			ptoks.append(operands[0:n])
			operands = operands[n:]

		# punctuation
		elif operands[0] in punc:
			ptoks.append(operands[0])
			operands = operands[1:]

		# register lists {...}
		elif operands[0] == '{':
			n = 1
			while operands[n:] and not operands[n] == '}':
				n += 1
			ptoks.append(operands[0:n+1])
			operands = operands[n+1:]

		# spaces discarded
		elif operands[0] == ' ':
			operands = operands[1:]
		else:
			raise Exception('pretokenize stumped on: -%s- (original input: %s)' % (operands, orig))

	#print 'pre tokens: ' + ' '.join(ptoks)

	#
	# real-tokenize
	#
	alias2gpr = {'sb':9,'sl':10,'fp':11,'ip':12,'sp':13,'lr':14,'pc':15}
	shifts = ['lsl','lsr','asr','rrx','ror']
	toks = [opcode]
	for tok in ptoks:
		if re.match(r'^[acs]psr_?.*', tok):
			toks.append('STATR')
		elif re.match(r'^r\d+', tok):
			toks.append('GPR')
		elif re.match(r'^q\d+', tok):
			toks.append('QREG')
		elif re.match(r'^d\d+', tok):
			toks.append('DREG')
		elif re.match(r'^p\d+', tok):
			toks.append('PREG')
		elif re.match(r'^c\d+', tok):
			toks.append('CREG')
		elif re.match(r'^s\d+', tok):
			toks.append('SREG')
		elif tok in alias2gpr:
			toks.append('GPR')
		elif re.match(r'^mvfr\d', tok):
			toks.append('MEDIAREG')
		elif tok in ['none', 'a', 'i', 'f', 'ai', 'af', 'if', 'aif']:
			toks.append('IRQ')
		elif tok in shifts:
			toks.append('SHIFT')
		elif re.match(r'^{.*?}', tok):
			toks.append('RLIST')
		elif re.match(r'^#?-?0x[a-fA-F0-9]+', tok):
			toks.append('NUM')
		elif re.match(r'^#?-?\d+', tok):
			toks.append('NUM')
		elif tok in list('[](),.*+-!^:') + ['CC','le','be']:
			toks.append(tok)
		elif tok in ['.s8','.s16','.s32','.s64','.u32','.8','.16','.32','.i32','.i64']:
			toks.append(tok)
		elif re.match(r'^[aif]+', tok):
			toks.append('OPT')
		elif tok in ['sy','st','ish','ishst','nsh','nshst','osh','oshst']: # dmb options
			toks.append('OPT')
		else:
			raise Exception('tokenize stumped on: -%s- (original input: %s)' % (tok, orig))

	# done
	return toks

gofer = None
cbuf = None
def disasm(insword):
	# initialize disassembler, if necessary
	global gofer, cbuf
	if not gofer:
		gofer = ctypes.CDLL("gofer.so")
		cbuf = ctypes.create_string_buffer(256)

	data = struct.pack('<I', insword)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))

	#print 'disassembled %08X to -%s-' % (insword, cbuf.value)
	return cbuf.value

def syntax_from_string(instr, insword):
	tokens = tokenize(instr, insword)
	if len(tokens) == 0:
		return tokens[0]
	else:
		return tokens[0] + ' ' + ''.join(tokens[1:])

def syntax_from_insword(insword):
	distxt = disasm(insword)
	if distxt == 'undef':
		return None
	return syntax_from_string(distxt, insword)

# return all 32-bit values that have 4, 3, 2, 1 and 0 bits set
def fuzz4():
	fuzz = [0]

	for positions in itertools.combinations(range(32), 1):
		mask = (1<<positions[0])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 2):
		mask = (1<<positions[0])|(1<<positions[1])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 3):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])
		fuzz.append(mask)

	for positions in itertools.combinations(range(32), 4):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])
		fuzz.append(mask)

	# fuzz should have all 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz

def fuzz5():
	fuzz = fuzz4()

	for positions in itertools.combinations(range(32), 5):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])|(1<<positions[4])
		fuzz.append(mask)

	# fuzz should have all 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29*28/120 + 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz

def fuzz6():
	fuzz = fuzz5()

	for positions in itertools.combinations(range(32), 6):
		mask = (1<<positions[0])|(1<<positions[1])|(1<<positions[2])|(1<<positions[3])|(1<<positions[4])|(1<<positions[5])
		fuzz.append(mask)

	# fuzz should have all 5-bit subets, 4-bit subsets, 3-bit subsets, 2-bit, 1-bit, 0-bit
	assert len(fuzz) == 32*31*30*29*28*27/720 + 32*31*30*29*28/120 + 32*31*30*29/24 + 32*31*30/6 + 32*31/2 + 32 + 1

	return fuzz
