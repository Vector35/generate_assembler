#!/usr/bin/env python

import re
import sys
import ctypes
import struct

import itertools

re_root = r'(?P<root>.*?)'
re_s = r'(?P<s>\.s)?'													# set flags
re_cc = r'(?P<cc>(eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le))?'	# condition code
#re_q = r'(?P<q>(\.w|\.n))?'											# qualifier (wide or narrow)
re_ds1 = r'(?P<ds1>(\.\w?\d+))?'										# data size
re_ds2 = r'(?P<ds2>(\.\w?\d+))?'										# data size

# this is built from opc_roots.py
# 372 roots
opc_roots = {'adc', 'add', 'adr', 'and', 'asr', 'b', 'bfc', 'bfi', 'bic', 'bkpt',
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
#
# return [opc_root, cc, opc_wildcc, opc_full, operands]
def distxt_split(arg, insword=0):
	# normalize to distxt (opcode and [operands])
	distxt = arg
	if type(arg) == int:
		distxt = disasm_libbinaryninjacore(arg)

	# split fully-qualified opcode and operands
	[opc_full, operands] = ['','']
	tmp = distxt.split(' ',1)
	opc_full = tmp[0]
	if tmp[1:]: operands = tmp[1]

	regex = r'^' + re_root + re_s + re_cc + re_ds1 + re_ds2 + r'$'
	m = re.match(regex, opc_full)
	if not m: raise Exception('regex no matchy %08X: %s' % (insword,opc_full))

	# determine opcode root
	#      is root+cc an opcode root? eg: "vc"+"ge", is "vcge" an opcode root?
	# else is root    an opcode root? eg: "vc",      is "vc" an opcode root?
	opc_root = None

	opc_cc = None
	tmp = m['root'] + (m['cc'] or '')
	if tmp in opc_roots:
		opc_root = tmp
		opc_cc = ''
	else:
		tmp = m['root']
		if tmp in opc_roots:
			opc_root = tmp
			opc_cc = m['cc'] or ''
		else:
			raise Exception('unfamiliar opcode %08X: %s' % (insword,opc_full))

	#
	opc_wildcc = opc_root + (m['s'] or '') + ('<c>' if opc_cc else '') + (m['ds1'] or '')  + (m['ds2'] or '')
	
	return (opc_root, opc_cc, opc_wildcc, opc_full, operands)

#------------------------------------------------------------------------------
# tokenizing, syntaxing
#------------------------------------------------------------------------------

def tokenize(distxt, insword=0):
	# split into opcode, operands, etc.
	(opc_root, opc_cc, opc_wildcc, opc_full, operands) = distxt_split(distxt)

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
			raise Exception('pretokenize stumped on: -%s- (original input: %08X:%s)' % (operands, insword, distxt))

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
	toks = [opc_wildcc]
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
			if i != len(ptoks)-1 and ptoks[i+1]!=']':
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

def syntax_from_distxt(distxt, insword=0):
	tokens = tokenize(distxt, insword)
	if len(tokens) == 1:
		return tokens[0]
	else:
		return tokens[0] + ' ' + ''.join(tokens[1:])

def syntax_from_insword(insword):
	distxt = disasm_libbinaryninjacore(insword)
	return syntax_from_distxt(distxt, insword)

#------------------------------------------------------------------------------
# disassembling
#------------------------------------------------------------------------------

g_arch = None
def disasm_python(insword):
	# initialize disassembler, if necessary
	global g_arch

	if not g_arch:
		# ensure binja in path, eg:
		# PATHPATH=/Users/andrewl/repos/vector35/binaryninja/ui/binaryninja.app/Contents/Resources/python/
		import binaryninja
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

#------------------------------------------------------------------------------
# fuzzing
#------------------------------------------------------------------------------

def test_check(actual, expected):
	if actual != expected:
		print('TEST FAILED!')
		print('  actual: %s' % str(actual))
		print('expected: %s' % str(expected))
		sys.exit(-1)	

if __name__ == '__main__':
	# basic instruction splitting
	expected = ('vmov','le','vmov<c>.f64','vmovle.f64','d0, #2.000000e+00')
	actual = distxt_split('vmovle.f64 d0, #2.000000e+00')
	test_check(actual, expected)

	expected = ('vmov','le','vmov<c>.f64','vmovle.f64','d0, #2.000000e+00')
	actual = distxt_split(0xDEB00B00)
	test_check(actual, expected)

	# temptation to take trailing two characters as CC (but they're not)
	expected = ('hvc','','hvc','hvc','')
	actual = distxt_split('hvc')
	test_check(actual, expected)

	expected = ('mls','','mls','mls','')
	actual = distxt_split('mls')
	test_check(actual, expected)

	expected = ('smmls','','smmls','smmls','')
	actual = distxt_split('smmls')
	test_check(actual, expected)

	expected = ('svc','','svc','svc','')
	actual = distxt_split('svc')
	test_check(actual, expected)

	expected = ('teq','','teq','teq','')
	actual = distxt_split('teq')
	test_check(actual, expected)

	expected = ('vacge','','vacge.f32','vacge.f32','')
	actual = distxt_split('vacge.f32')
	test_check(actual, expected)

	expected = ('vacgt','','vacgt.f32','vacgt.f32','')
	actual = distxt_split('vacgt.f32')
	test_check(actual, expected)

	expected = ('vceq','','vceq.f32','vceq.f32','')
	actual = distxt_split('vceq.f32')
	test_check(actual, expected)

	expected = ('vceq','','vceq.i16','vceq.i16','')
	actual = distxt_split('vceq.i16')
	test_check(actual, expected)

	expected = ('vceq','','vceq.i32','vceq.i32','')
	actual = distxt_split('vceq.i32')
	test_check(actual, expected)

	expected = ('vceq','','vceq.i64','vceq.i64','')
	actual = distxt_split('vceq.i64')
	test_check(actual, expected)

	expected = ('vceq','','vceq.i8','vceq.i8','')
	actual = distxt_split('vceq.i8')
	test_check(actual, expected)

	expected = ('vcge','','vcge.f32','vcge.f32','')
	actual = distxt_split('vcge.f32')
	test_check(actual, expected)

	expected = ('vcge','','vcge.s16','vcge.s16','')
	actual = distxt_split('vcge.s16')
	test_check(actual, expected)

	expected = ('vcge','','vcge.s32','vcge.s32','')
	actual = distxt_split('vcge.s32')
	test_check(actual, expected)

	expected = ('vcge','','vcge.s64','vcge.s64','')
	actual = distxt_split('vcge.s64')
	test_check(actual, expected)

	expected = ('vcge','','vcge.s8','vcge.s8','')
	actual = distxt_split('vcge.s8')
	test_check(actual, expected)

	expected = ('vcge','','vcge.u16','vcge.u16','')
	actual = distxt_split('vcge.u16')
	test_check(actual, expected)

	expected = ('vcge','','vcge.u32','vcge.u32','')
	actual = distxt_split('vcge.u32')
	test_check(actual, expected)

	expected = ('vcge','','vcge.u64','vcge.u64','')
	actual = distxt_split('vcge.u64')
	test_check(actual, expected)

	expected = ('vcge','','vcge.u8','vcge.u8','')
	actual = distxt_split('vcge.u8')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.f32','vcgt.f32','')
	actual = distxt_split('vcgt.f32')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.s16','vcgt.s16','')
	actual = distxt_split('vcgt.s16')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.s32','vcgt.s32','')
	actual = distxt_split('vcgt.s32')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.s64','vcgt.s64','')
	actual = distxt_split('vcgt.s64')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.s8','vcgt.s8','')
	actual = distxt_split('vcgt.s8')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.u16','vcgt.u16','')
	actual = distxt_split('vcgt.u16')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.u32','vcgt.u32','')
	actual = distxt_split('vcgt.u32')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.u64','vcgt.u64','')
	actual = distxt_split('vcgt.u64')
	test_check(actual, expected)

	expected = ('vcgt','','vcgt.u8','vcgt.u8','')
	actual = distxt_split('vcgt.u8')
	test_check(actual, expected)

	expected = ('vcle','','vcle.f32','vcle.f32','')
	actual = distxt_split('vcle.f32')
	test_check(actual, expected)

	expected = ('vcle','','vcle.s16','vcle.s16','')
	actual = distxt_split('vcle.s16')
	test_check(actual, expected)

	expected = ('vcle','','vcle.s32','vcle.s32','')
	actual = distxt_split('vcle.s32')
	test_check(actual, expected)

	expected = ('vcle','','vcle.s8','vcle.s8','')
	actual = distxt_split('vcle.s8')
	test_check(actual, expected)

	expected = ('vcls','','vcls.s16','vcls.s16','')
	actual = distxt_split('vcls.s16')
	test_check(actual, expected)

	expected = ('vcls','','vcls.s32','vcls.s32','')
	actual = distxt_split('vcls.s32')
	test_check(actual, expected)

	expected = ('vcls','','vcls.s8','vcls.s8','')
	actual = distxt_split('vcls.s8')
	test_check(actual, expected)

	expected = ('vclt','','vclt.f32','vclt.f32','')
	actual = distxt_split('vclt.f32')
	test_check(actual, expected)

	expected = ('vclt','','vclt.s16','vclt.s16','')
	actual = distxt_split('vclt.s16')
	test_check(actual, expected)

	expected = ('vclt','','vclt.s32','vclt.s32','')
	actual = distxt_split('vclt.s32')
	test_check(actual, expected)

	expected = ('vclt','','vclt.s8','vclt.s8','')
	actual = distxt_split('vclt.s8')
	test_check(actual, expected)

	expected = ('vmls','','vmls.f32','vmls.f32','')
	actual = distxt_split('vmls.f32')
	test_check(actual, expected)

	expected = ('vmls','','vmls.f64','vmls.f64','')
	actual = distxt_split('vmls.f64')
	test_check(actual, expected)

	expected = ('vmls','','vmls.i16','vmls.i16','')
	actual = distxt_split('vmls.i16')
	test_check(actual, expected)

	expected = ('vmls','','vmls.i32','vmls.i32','')
	actual = distxt_split('vmls.i32')
	test_check(actual, expected)

	expected = ('vmls','','vmls.i64','vmls.i64','')
	actual = distxt_split('vmls.i64')
	test_check(actual, expected)

	expected = ('vmls','','vmls.i8','vmls.i8','')
	actual = distxt_split('vmls.i8')
	test_check(actual, expected)

	expected = ('vnmls','','vnmls.f32','vnmls.f32','')
	actual = distxt_split('vnmls.f32')
	test_check(actual, expected)

	expected = ('vnmls','','vnmls.f64','vnmls.f64','')
	actual = distxt_split('vnmls.f64')
	test_check(actual, expected)

	# now actually split out the condition code
	expected = ('adc','eq','adc.s<c>','adc.seq','r0, r0, r0')
	actual = distxt_split('adc.seq r0, r0, r0')
	test_check(actual, expected)

	# calculate syntax given disassembly text
	expected = 'adc.s<c> GPR,GPR,GPR'
	actual = syntax_from_distxt('adc.shs r0, r0, r0')
	test_check(actual, expected)

	actual = syntax_from_insword(0x20b00000)
	test_check(actual, expected)
