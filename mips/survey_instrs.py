#!/usr/bin/env python

import struct
import ctypes
import random

gofer = ctypes.CDLL("gofer.so")
cbuf = ctypes.create_string_buffer(256)

while 1:
	instrWord = random.randint(0,0xFFFFFFFF)
	data = struct.pack('<I', instrWord)
	gofer.get_disasm_capstone(data, 4, ctypes.byref(cbuf))
	print '%08X: %s' % (instrWord, cbuf.value)

