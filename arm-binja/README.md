ARM assembler with [capstone](https://github.com/aquynh/capstone) as the oracle.

## Notes
* "root" opcode is the opcode without any "s" (capstone) or ".s" (binja) or condition codes or size suffixes
* "fully-qualified" opcode is opcode with all that shit
* "wildcc" opcode is the fully-qualified one with condition code (if it exists) substituted with "<c>"
