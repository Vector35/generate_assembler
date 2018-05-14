import re
import struct
import ctypes

def tokenize(string):
	string_ = string
	#print 'tokenizing: %s' % string
	result = []

	# pick off opcode
	opcode = re.match(r'^(\w+)', string).group(1)
	result.append(['OPC', opcode])
	string = string[len(opcode):]

	# pick off the rest
	while string:
		eat = 0
		if string.startswith(' '):
			eat = 1
		elif re.match(r'^r\d+', string):
			tmp = re.match(r'^(r\d+)', string).group(1) # eg: r0
			result.append( ('GPR',int(tmp[1:])) )
			eat = len(tmp)
		elif re.match(r'^v\d+', string):
			tmp = re.match(r'^(v\d+)', string).group(1) # eg: v0
			result.append( ('VREG',int(tmp[1:])) )
			eat = len(tmp)
		elif len(string) >= 2 and string[0:2] in ['lt','gt','eq','so'] and \
		  (len(string)==2 or (re.match(r'[^\w]',string[2]))):
			result.append( ('FLAG',string[0:2]) )
			eat = 2
		elif re.match(r'^cr\d+', string):
			tmp = re.match(r'^(cr\d+)', string).group(1) # eg: cr1
			result.append( ('CREG',int(tmp[2:])) )
			eat = len(tmp)
		elif re.match(r'^vs\d+', string):
			tmp = re.match(r'^(vs\d+)', string).group(1) # eg: vs0
			result.append( ('VSREG',int(tmp[2:])) )
			eat = len(tmp)			
		elif re.match(r'^f\d+', string):
			tmp = re.match(r'^(f\d+)', string).group(1) # eg: f0
			result.append( ('FREG',int(tmp[1:])) )
			eat = len(tmp)
		elif re.match(r'^-?0x[a-fA-F0-9]+', string):
			tmp = re.match(r'^(-?0x[a-fA-F0-9]+)', string).group(1)
			result.append( ('NUM', int(tmp,16)) )
			eat = len(tmp)
		elif re.match(r'^-?\d+', string):
			tmp = re.match(r'(^-?\d+)', string).group(1)
			result.append( ('NUM', int(tmp)) )
			eat = len(tmp)
		elif string[0] in list('(),.*+-'):
			result.append( (string[0],string[0]) )
			eat = 1
		else:
			raise Exception('dunno what to do with: %s (original input: %s)' % (string, string_))
		
		string = string[eat:]

	return result

def tokenize_types(string):
	tokens = tokenize(string)
	types, values = zip(*tokens)
	return ' '.join((values[0],) + types[1:])

(adapt, cbuf, ibuf) = (None, None, None)
def disasm(word):
	global adapt, cbuf, ibuf

	if not adapt:
		adapt = ctypes.CDLL("testadapt.so")
		cbuf = ctypes.create_string_buffer(256)
		ibuf = ctypes.create_string_buffer(4)

	# form input buffer
	data = struct.pack('<I', word)

	# ask capstone
	rc = adapt.get_disasm_capstone(data, 4, ctypes.byref(cbuf))
	if rc: raise Exception("ERROR: get_disasm_capstone()")
	return cbuf.value
