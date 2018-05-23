# notes
mips sometimes has opcode encoded on LHS, sometimes RHS, see xor and xori
mips has FPU fmt mnemonics like ".s", ".d", ".w" ".l" and ".ps", see A.3 Floating Point Unit Instruction Format Encodings
mips opcode is sometimes way distant from opcode.X, for example srl is 00000002 but srl.d is 796FF04D
  this is main reason why I include fmt in the opcode
