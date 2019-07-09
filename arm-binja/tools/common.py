import re
import ctypes
import struct

import itertools

# ensure binja in path, eg:
# PATHPATH=/Users/andrewl/repos/vector35/binaryninja/ui/binaryninja.app/Contents/Resources/python/
import binaryninja

re_root = r'(?P<root>.*?)'
re_s = r'(?P<s>\.s)?'													# set flags
re_cc = r'(?P<cc>(eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le))?'	# condition code
#re_q = r'(?P<q>(\.w|\.n))?'											# qualifier (wide or narrow)
re_ds1 = r'(?P<ds1>(\.\w?\d+))?'										# data size
re_ds2 = r'(?P<ds2>(\.\w?\d+))?'										# data size

# this is built from opcodes.py
opcodes = {'adc', 'add', 'adr', 'and', 'asr', 'b', 'bfc', 'bfi', 'bic', 'bkpt',
'bl', 'blx', 'bx', 'bxj', 'cdp', 'cdp2', 'clrex', 'clz', 'cmn', 'cmp', 'cps',
'cpsid', 'cpsie', 'dbg', 'dmb', 'dsb', 'eor', 'eret', 'fldmdbx', 'fldmiax',
'fstmdbx', 'fstmiax', 'hint', 'hvc', 'isb', 'ldc', 'ldc2', 'ldc2l', 'ldcl',
'ldm', 'ldmda', 'ldmdb', 'ldmib', 'ldr', 'ldrb', 'ldrbt', 'ldrd', 'ldrex',
'ldrexb', 'ldrexd', 'ldrexh', 'ldrh', 'ldrht', 'ldrsb', 'ldrsbt', 'ldrsh',
'ldrsht', 'ldrt', 'lsl', 'lsr', 'mcr', 'mcr2', 'mcrr', 'mcrr2', 'mla', 'mls',
'mov', 'movt', 'movw', 'mrc', 'mrc2', 'mrrc', 'mrrc2', 'mrs', 'msr', 'mul',
'mvn', 'nop', 'orr', 'pkhbt', 'pkhtb', 'pld', 'pldw', 'pli', 'pop', 'push',
'qadd', 'qadd16', 'qadd8', 'qasx', 'qdadd', 'qdsub', 'qsax', 'qsub', 'qsub16',
'qsub8', 'rbit', 'rev', 'rev16', 'revsh', 'rfeda', 'rfedb', 'rfeia', 'rfeib',
'ror', 'rrx', 'rsb', 'rsc', 'sadd16', 'sadd8', 'sasx', 'sbc', 'sbfx', 'sdiv',
'sel', 'setend', 'sev', 'shadd16', 'shadd8', 'shasx', 'shsax', 'shsub16',
'shsub8', 'smc', 'smlabb', 'smlabt', 'smlad', 'smladx', 'smlal', 'smlalbb',
'smlalbt', 'smlald', 'smlaldx', 'smlaltb', 'smlaltt', 'smlatb', 'smlatt',
'smlawb', 'smlawt', 'smlsd', 'smlsdx', 'smlsld', 'smlsldx', 'smmla', 'smmlar',
'smmls', 'smmlsr', 'smmul', 'smmulr', 'smuad', 'smuadx', 'smulbb', 'smulbt',
'smull', 'smultb', 'smultt', 'smulwb', 'smulwt', 'smusd', 'smusdx', 'srsda',
'srsdb', 'srsia', 'srsib', 'ssat', 'ssat16', 'ssax', 'ssub16', 'ssub8', 'stc',
'stc2', 'stc2l', 'stcl', 'stm', 'stmda', 'stmdb', 'stmib', 'str', 'strb',
'strbt', 'strd', 'strex', 'strexb', 'strexd', 'strexh', 'strh', 'strht',
'strt', 'sub', 'svc', 'swp', 'swpb', 'sxtab', 'sxtab16', 'sxtah', 'sxtb',
'sxtb16', 'sxth', 'teq', 'tst', 'uadd16', 'uadd8', 'uasx', 'ubfx', 'udf',
'udiv', 'uhadd16', 'uhadd8', 'uhasx', 'uhsax', 'uhsub16', 'uhsub8', 'umaal',
'umlal', 'umull', 'uqadd16', 'uqadd8', 'uqasx', 'uqsax', 'uqsub16', 'uqsub8',
'usad8', 'usada8', 'usat', 'usat16', 'usax', 'usub16', 'usub8', 'uxtab',
'uxtab16', 'uxtah', 'uxtb', 'uxtb16', 'uxth', 'vaba', 'vabal', 'vabd', 'vabdl',
'vabs', 'vacge', 'vacgt', 'vadd', 'vaddhn', 'vaddl', 'vaddw', 'vand', 'vbic',
'vbif', 'vbit', 'vbsl', 'vceq', 'vcge', 'vcgt', 'vcle', 'vcls', 'vclt', 'vclz',
'vcmp', 'vcmpe', 'vcnt', 'vcvt', 'vcvtb', 'vcvtr', 'vcvtt', 'vdiv', 'vdup',
'veor', 'vext', 'vfma', 'vfms', 'vfnma', 'vfnms', 'vhadd', 'vhsub', 'vld1',
'vld2', 'vld3', 'vld4', 'vldmdb', 'vldmia', 'vldr', 'vmax', 'vmin', 'vmla',
'vmlal', 'vmls', 'vmlsl', 'vmov', 'vmovl', 'vmovn', 'vmrs', 'vmsr', 'vmul',
'vmull', 'vmvn', 'vneg', 'vnmla', 'vnmls', 'vnmul', 'vorn', 'vorr', 'vpadal',
'vpadd', 'vpaddl', 'vpmax', 'vpmin', 'vpop', 'vpush', 'vqabs', 'vqadd',
'vqdmlal', 'vqdmlsl', 'vqdmulh', 'vqdmull', 'vqmovn', 'vqmovun', 'vqneg',
'vqrdmulh', 'vqrshl', 'vqrshrn', 'vqrshrun', 'vqshl', 'vqshlu', 'vqshrn',
'vqshrun', 'vqsub', 'vraddhn', 'vrecpe', 'vrecps', 'vrev16', 'vrev32',
'vrev64', 'vrhadd', 'vrintr', 'vrintx', 'vrintz', 'vrshl', 'vrshr', 'vrshrn',
'vrsqrte', 'vrsqrts', 'vrsra', 'vrsubhn', 'vshl', 'vshll', 'vshr', 'vshrn',
'vsli', 'vsqrt', 'vsra', 'vsri', 'vst1', 'vst2', 'vst3', 'vst4', 'vstmdb',
'vstmia', 'vstr', 'vsub', 'vsubhn', 'vsubl', 'vsubw', 'vswp', 'vtbl', 'vtbx',
'vtrn', 'vtst', 'vuzp', 'vzip', 'wfe', 'wfi', 'yield'}

#------------------------------------------------------------------------------
# opcode extraction functions
#------------------------------------------------------------------------------

# convert fully-qualified opcode (with ".s", CC, .32, etc.) to opcode
# eg: "3E000B40 vmlslo.f64" to "vmls"
# eg: "E75000D0 smmls" to "smmls" (see possibility "ls" maken for CC)
def opcode_from_fqo(fqo, insword=0):
	regex = r'^' + re_root + re_s + re_cc + re_ds1 + re_ds2 + r'$'
	#m = re.match(regex, fqo).groupdict()
	m = re.match(regex, fqo)
	if not m: raise Exception('regex no matchy %08X: %s' % (insword,fqo))

	# try root + candidate CC as opcode
	tmp = m['root'] + (m['cc'] or '')
	if tmp in opcodes:
		return tmp

	# try root (without CC)
	tmp = m['root']
	if tmp in opcodes:
		return tmp

	raise Exception('unfamiliar opcode %08X: %s' % (insword,fqo))

def opcode_from_distxt(distxt, insword=0):
	fqo = distxt.split(' ',1)[0]
	return opcode_from_fqo(fqo, insword)

def opcode_from_insword(insword):
	distxt = disasm_libbinaryninjacore(insword)
	return opcode_from_distxt(distxt, insword)

#------------------------------------------------------------------------------
# tokenizing, syntaxing
#------------------------------------------------------------------------------

def tokenize(distxt, insword):
	orig = distxt

	# opcode, operands
	[opcode, fqo, operands] = ['', '', '']
	if ' ' in distxt:
		i = distxt.index(' ')
		fqo = distxt[0:i]
		opcode = opcode_from_fqo(fqo, insword)
		operands = distxt[i+1:]
	else:
		fqo = distxt
		opcode = opcode_from_fqo(distxt, insword)
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
			raise Exception('pretokenize stumped on: -%s- (original input: %08X:%s)' % (operands, insword, orig))

	#print 'pre tokens: ' + ' '.join(ptoks)

	#
	# real-tokenize
	#
	alias2gpr = {'sb':9,'sl':10,'fp':11,'ip':12,'sp':13,'lr':14,'pc':15}
	shifts = ['lsl','lsr','asr','rrx','ror']
	banked_regs = [	"elr_hyp",
		"lr_abt", "lr_fiq", "lr_irq", "lr_mon", "lr_svc", "lr_und", "lr_usr",
		"r10_fiq", "r10_usr", "r11_fiq", "r11_usr", "r12_fiq", "r12_usr", "r8_fiq",
		"r8_usr", "r9_fiq", "r9_usr", "spsr_abt", "spsr_fiq", "spsr_hyp",
		"spsr_irq", "spsr_mon", "spsr_svc", "spsr_und", "sp_abt", "sp_fiq",
		"sp_hyp", "sp_irq", "sp_mon", "sp_svc", "sp_und", "sp_usr"]
	toks = [opcode]
	for (i,tok) in enumerate(ptoks):
		if re.match(r'^[acs]psr_?.*', tok):
			toks.append('STATR')
		elif tok in banked_regs:
			toks.append('BREG')
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
			if i != len(ptoks)-1:
				toks.append(' ')
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
			raise Exception('tokenize stumped on: -%s- (original input: %08X:%s)' % (tok, insword, orig))

	# done
	return toks

def syntax_from_distxt(distxt, insword):
	tokens = tokenize(distxt, insword)
	if len(tokens) == 1:
		return tokens[0]
	else:
		return tokens[0] + ' ' + ''.join(tokens[1:])

def syntax_from_insword(insword):
	distxt = disasm_libbinaryninjacore(insword)
	return syntax_from_string(distxt, insword)

#------------------------------------------------------------------------------
# disassembling
#------------------------------------------------------------------------------

g_arch = None
def disasm_python(insword):
	# initialize disassembler, if necessary
	global g_arch

	if not g_arch:
		g_arch = binaryninja.Architecture['armv7']
	

	data = struct.pack('<I', insword)
	toksAndLen = g_arch.get_instruction_text(data, 0)
	toks = toksAndLen[0]
	strs = map(lambda x: x.text, toks)
	return ''.join(strs)

gofer = None
cbuf = None
def disasm_libbinaryninjacore(insword):
	# initialize disassembler, if necessary
	global gofer, cbuf
	if not gofer:
		gofer = ctypes.CDLL("gofer.so")
		cbuf = ctypes.create_string_buffer(256)

	data = struct.pack('<I', insword)
	gofer.get_disasm_binja(data, ctypes.byref(cbuf))

	#print 'disassembled %08X to -%s-' % (insword, cbuf.value)
	return cbuf.value.decode('utf-8')

#------------------------------------------------------------------------------
# fuzzing
#------------------------------------------------------------------------------

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
