Generate assemblers from disassemblers, code to accompany the [2018 Jailbreak Security Summit](http://www.jailbreaksecuritysummit.com/) talk.

The original two architecture tested for this talk were powerpc and mips. Additional architecture exist in their respective directories.

The powerpc assembler is live in Binary Ninja as part of the open source powerpc architecture, see: [https://github.com/Vector35/ppc-capstone](https://github.com/Vector35/ppc-capstone).

## Files

* ./ppc - ppc assembler
* ./ppc/tools - helper programs, tests, etc.
* ./mips - mips assembler
* ./mips/tools - helper programs, tests, etc.
* ./arm-capstone - arm assembler with capstone as oracle (WIP)
* ./arm-capstone/tools - helper programs, test, etc.
* ./arm-binja - arm assembler with binary ninja as oracle (WIP)
* ./arm-binja/tools - helper programs, test, etc.

## Speed

With compile optimized for speed, I'm getting 4k asssembles/sec for randomly sampled encodings over the 32-bit instruction space.

## How?

1. Travel all 32-bit encodings, disassemble, and collect opcode->encoding "seeds". See opc_seeds.cpp.
2. For each opcode->encoding seed, fuzz the encoding and tokenize the result, further breaking the instruction spaces into syntax->encoding seeds. See syn_seeds.py.
3. For each syn->encoding seed, fuzz again to get bit patterns like "0011xx01xx1" that instruct the genetic algorithm where to toggle bits. See syn_bitpats.py.
4. Genetic algorithm changes the wildcard x bits from the bit pattern, using the fitness function that considers the type of each token. String tokens are strcmp'd, numeric tokens are hamming-distance compared.

## License

This code MIT licensed, see [LICENSE.txt](./license.txt).

It links against the [Capstone disassembly framework](https://github.com/aquynh/capstone) which is BSD licensed.
