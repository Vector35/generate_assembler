Generate assemblers from disassemblers, code to accompany the [2018 Jailbreak Security Summit](http://www.jailbreaksecuritysummit.com/) talk.

* See the ./ppc/tools/README.md for ppc specific notes
* See the ./mips/tools/README.md for mips specific notes

## Files

* ./ppc - the main ppc assembler
* ./ppc/tools - helper programs, tests, etc.
* ./mips - the main mips assembler
* ./mips/tools - helper programs, tests, etc.
* ./arm - the main arm assembler (WIP)
* ./arm/tools - helper programs, test, etc.

## Speed

With compile optimized for speed, I'm getting 4k asssembles/sec for randomly sampled encodings over the 32-bit instruction space.

## How?

1. Travel all 32-bit encodings, disassemble, and collect opcode->encoding "seeds". See opc_seeds.cpp.
2. For each opcode->encoding seed, fuzz the encoding and tokenize the result, further breaking the instruction spaces into syntax->encoding seeds. See syn_seeds.py.
3. For each syn->encoding seed, fuzz again to get bit patterns like "0011xx01xx1" that instruct the genetic algorithm where to toggle bits. See syn_bitpats.py.
4. Genetic algorithm changes the wildcard x bits from the bit pattern, using the fitness function that considers the type of each token. String tokens are strcmp'd, numeric tokens are hamming-distance compared.
