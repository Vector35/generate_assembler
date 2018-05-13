
/* */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* c++ stuff */
#include <map>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* capstone stuff */
#include <capstone/capstone.h>

#define MYLOG printf
//#define MYLOG(...) while(0);

/*****************************************************************************/
/* precomputed stuff */
/*****************************************************************************/

struct info {
	uint32_t seed; /* start parent */
	uint32_t mask; /* which bits to mutate */
};

map<string, info> lookup = {
{                   "abs.d FREG , FREG",{0x46200005,0x0000F780}}, // 0100011000100000xxxx0xxxx0000101  abs.d $f0, $f0
{                   "abs.s FREG , FREG",{0x46000005,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx000101  abs.s $f0, $f0
{             "absq_s.ph GPREG , GPREG",{0x7C000252,0x001FF800}}, // 01111100000xxxxxxxxxx01001010010  absq_s.ph $zero, $zero
{             "absq_s.qb GPREG , GPREG",{0x7C000052,0x001FF800}}, // 01111100000xxxxxxxxxx00001010010  absq_s.qb $zero, $zero
{              "absq_s.w GPREG , GPREG",{0x7C000452,0x001FF800}}, // 01111100000xxxxxxxxxx10001010010  absq_s.w $zero, $zero
{           "add GPREG , GPREG , GPREG",{0x00000020,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100000  add $zero, $zero, $zero
{            "add.d FREG , FREG , FREG",{0x46200000,0x001EF780}}, // 01000110001xxxx0xxxx0xxxx0000000  add.d $f0, $f0, $f0
{            "add.s FREG , FREG , FREG",{0x46000000,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx000000  add.s $f0, $f0, $f0
{          "add_a.b WREG , WREG , WREG",{0x78000010,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx010000  add_a.b $w0, $w0, $w0
{          "add_a.d WREG , WREG , WREG",{0x78600010,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010000  add_a.d $w0, $w0, $w0
{          "add_a.h WREG , WREG , WREG",{0x78200010,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010000  add_a.h $w0, $w0, $w0
{          "add_a.w WREG , WREG , WREG",{0x78400010,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010000  add_a.w $w0, $w0, $w0
{           "addiu GPREG , GPREG , NUM",{0x24000000,0x03FFFFFF}}, // 001001xxxxxxxxxxxxxxxxxxxxxxxxxx  addiu $zero, $zero, 0
{                 "addiupc GPREG , NUM",{0xEC000000,0x03E7FFFF}}, // 111011xxxxx00xxxxxxxxxxxxxxxxxxx  addiupc $zero, 0
{       "addq.ph GPREG , GPREG , GPREG",{0x7C000290,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01010010000  addq.ph $zero, $zero, $zero
{     "addq_s.ph GPREG , GPREG , GPREG",{0x7C000390,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01110010000  addq_s.ph $zero, $zero, $zero
{      "addq_s.w GPREG , GPREG , GPREG",{0x7C000590,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10110010000  addq_s.w $zero, $zero, $zero
{      "addqh.ph GPREG , GPREG , GPREG",{0x7C000218,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01000011000  addqh.ph $zero, $zero, $zero
{       "addqh.w GPREG , GPREG , GPREG",{0x7C000418,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10000011000  addqh.w $zero, $zero, $zero
{    "addqh_r.ph GPREG , GPREG , GPREG",{0x7C000298,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01010011000  addqh_r.ph $zero, $zero, $zero
{     "addqh_r.w GPREG , GPREG , GPREG",{0x7C000498,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10010011000  addqh_r.w $zero, $zero, $zero
{         "adds_a.b WREG , WREG , WREG",{0x78800010,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx010000  adds_a.b $w0, $w0, $w0
{         "adds_a.d WREG , WREG , WREG",{0x78E00010,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010000  adds_a.d $w0, $w0, $w0
{         "adds_a.h WREG , WREG , WREG",{0x78A00010,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010000  adds_a.h $w0, $w0, $w0
{         "adds_a.w WREG , WREG , WREG",{0x78C00010,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010000  adds_a.w $w0, $w0, $w0
{         "adds_s.b WREG , WREG , WREG",{0x79000010,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx010000  adds_s.b $w0, $w0, $w0
{         "adds_s.d WREG , WREG , WREG",{0x79600010,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010000  adds_s.d $w0, $w0, $w0
{         "adds_s.h WREG , WREG , WREG",{0x79200010,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010000  adds_s.h $w0, $w0, $w0
{         "adds_s.w WREG , WREG , WREG",{0x79400010,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010000  adds_s.w $w0, $w0, $w0
{         "adds_u.b WREG , WREG , WREG",{0x79800010,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx010000  adds_u.b $w0, $w0, $w0
{         "adds_u.d WREG , WREG , WREG",{0x79E00010,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx010000  adds_u.d $w0, $w0, $w0
{         "adds_u.h WREG , WREG , WREG",{0x79A00010,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx010000  adds_u.h $w0, $w0, $w0
{         "adds_u.w WREG , WREG , WREG",{0x79C00010,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx010000  adds_u.w $w0, $w0, $w0
{         "addsc GPREG , GPREG , GPREG",{0x7C000410,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10000010000  addsc $zero, $zero, $zero
{          "addu GPREG , GPREG , GPREG",{0x00010021,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100001  addu $zero, $zero, $at
{       "addu.ph GPREG , GPREG , GPREG",{0x7C000210,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01000010000  addu.ph $zero, $zero, $zero
{       "addu.qb GPREG , GPREG , GPREG",{0x7C000010,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00000010000  addu.qb $zero, $zero, $zero
{     "addu_s.ph GPREG , GPREG , GPREG",{0x7C000310,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01100010000  addu_s.ph $zero, $zero, $zero
{     "addu_s.qb GPREG , GPREG , GPREG",{0x7C000110,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00100010000  addu_s.qb $zero, $zero, $zero
{      "adduh.qb GPREG , GPREG , GPREG",{0x7C000018,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00000011000  adduh.qb $zero, $zero, $zero
{    "adduh_r.qb GPREG , GPREG , GPREG",{0x7C000098,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00010011000  adduh_r.qb $zero, $zero, $zero
{           "addv.b WREG , WREG , WREG",{0x7800000E,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx001110  addv.b $w0, $w0, $w0
{           "addv.d WREG , WREG , WREG",{0x7860000E,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx001110  addv.d $w0, $w0, $w0
{           "addv.h WREG , WREG , WREG",{0x7820000E,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx001110  addv.h $w0, $w0, $w0
{           "addv.w WREG , WREG , WREG",{0x7840000E,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx001110  addv.w $w0, $w0, $w0
{           "addvi.b WREG , WREG , NUM",{0x78000006,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx000110  addvi.b $w0, $w0, 0
{           "addvi.d WREG , WREG , NUM",{0x78600006,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx000110  addvi.d $w0, $w0, 0
{           "addvi.h WREG , WREG , NUM",{0x78200006,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx000110  addvi.h $w0, $w0, 0
{           "addvi.w WREG , WREG , NUM",{0x78400006,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx000110  addvi.w $w0, $w0, 0
{         "addwc GPREG , GPREG , GPREG",{0x7C000450,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10001010000  addwc $zero, $zero, $zero
{   "align GPREG , GPREG , GPREG , NUM",{0x7C000220,0x03FFF8C0}}, // 011111xxxxxxxxxxxxxxx010xx100000  align $zero, $zero, $zero, 0
{                  "aluipc GPREG , NUM",{0xEC1F0000,0x03E0FFFF}}, // 111011xxxxx11111xxxxxxxxxxxxxxxx  aluipc $zero, 0
{           "and GPREG , GPREG , GPREG",{0x00000024,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100100  and $zero, $zero, $zero
{            "and.v WREG , WREG , WREG",{0x7800001E,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx011110  and.v $w0, $w0, $w0
{            "andi GPREG , GPREG , NUM",{0x30000000,0x03FFFFFF}}, // 001100xxxxxxxxxxxxxxxxxxxxxxxxxx  andi $zero, $zero, 0
{            "andi.b WREG , WREG , NUM",{0x78000000,0x00FFFFC0}}, // 01111000xxxxxxxxxxxxxxxxxx000000  andi.b $w0, $w0, 0
{          "append GPREG , GPREG , NUM",{0x7C000031,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00000110001  append $zero, $zero, 0
{         "asub_s.b WREG , WREG , WREG",{0x7A000011,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx010001  asub_s.b $w0, $w0, $w0
{         "asub_s.d WREG , WREG , WREG",{0x7A600011,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010001  asub_s.d $w0, $w0, $w0
{         "asub_s.h WREG , WREG , WREG",{0x7A200011,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010001  asub_s.h $w0, $w0, $w0
{         "asub_s.w WREG , WREG , WREG",{0x7A400011,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010001  asub_s.w $w0, $w0, $w0
{         "asub_u.b WREG , WREG , WREG",{0x7A800011,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx010001  asub_u.b $w0, $w0, $w0
{         "asub_u.d WREG , WREG , WREG",{0x7AE00011,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010001  asub_u.d $w0, $w0, $w0
{         "asub_u.h WREG , WREG , WREG",{0x7AA00011,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010001  asub_u.h $w0, $w0, $w0
{         "asub_u.w WREG , WREG , WREG",{0x7AC00011,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010001  asub_u.w $w0, $w0, $w0
{             "aui GPREG , GPREG , NUM",{0x3C000000,0x03FFFFFF}}, // 001111xxxxxxxxxxxxxxxxxxxxxxxxxx  aui $zero, $zero, 0
{                   "auipc GPREG , NUM",{0xEC1E0000,0x03E0FFFF}}, // 111011xxxxx11110xxxxxxxxxxxxxxxx  auipc $zero, 0
{          "ave_s.b WREG , WREG , WREG",{0x7A000010,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx010000  ave_s.b $w0, $w0, $w0
{          "ave_s.d WREG , WREG , WREG",{0x7A600010,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010000  ave_s.d $w0, $w0, $w0
{          "ave_s.h WREG , WREG , WREG",{0x7A200010,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010000  ave_s.h $w0, $w0, $w0
{          "ave_s.w WREG , WREG , WREG",{0x7A400010,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010000  ave_s.w $w0, $w0, $w0
{          "ave_u.b WREG , WREG , WREG",{0x7A800010,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx010000  ave_u.b $w0, $w0, $w0
{          "ave_u.d WREG , WREG , WREG",{0x7AE00010,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010000  ave_u.d $w0, $w0, $w0
{          "ave_u.h WREG , WREG , WREG",{0x7AA00010,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010000  ave_u.h $w0, $w0, $w0
{          "ave_u.w WREG , WREG , WREG",{0x7AC00010,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010000  ave_u.w $w0, $w0, $w0
{         "aver_s.b WREG , WREG , WREG",{0x7B000010,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx010000  aver_s.b $w0, $w0, $w0
{         "aver_s.d WREG , WREG , WREG",{0x7B600010,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx010000  aver_s.d $w0, $w0, $w0
{         "aver_s.h WREG , WREG , WREG",{0x7B200010,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx010000  aver_s.h $w0, $w0, $w0
{         "aver_s.w WREG , WREG , WREG",{0x7B400010,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx010000  aver_s.w $w0, $w0, $w0
{         "aver_u.b WREG , WREG , WREG",{0x7B800010,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx010000  aver_u.b $w0, $w0, $w0
{         "aver_u.d WREG , WREG , WREG",{0x7BE00010,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx010000  aver_u.d $w0, $w0, $w0
{         "aver_u.h WREG , WREG , WREG",{0x7BA00010,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx010000  aver_u.h $w0, $w0, $w0
{         "aver_u.w WREG , WREG , WREG",{0x7BC00010,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx010000  aver_u.w $w0, $w0, $w0
{                               "b NUM",{0x10000000,0x0000FFFF}}, // 0001000000000000xxxxxxxxxxxxxxxx  b 4
{                             "bal NUM",{0x04110000,0x0000FFFF}}, // 0000010000010001xxxxxxxxxxxxxxxx  bal 4
{                            "balc NUM",{0xE8000000,0x03FFFFFF}}, // 111010xxxxxxxxxxxxxxxxxxxxxxxxxx  balc 0
{          "balign GPREG , GPREG , NUM",{0x7C000431,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10000110001  balign $zero, $zero, 0
{                              "bc NUM",{0xC8000000,0x03FFFFFF}}, // 110010xxxxxxxxxxxxxxxxxxxxxxxxxx  bc 0
{                   "bc1eqz FREG , NUM",{0x45200000,0x001FFFFF}}, // 01000101001xxxxxxxxxxxxxxxxxxxxx  bc1eqz $f0, 4
{                   "bc1nez FREG , NUM",{0x45A00000,0x001FFFFF}}, // 01000101101xxxxxxxxxxxxxxxxxxxxx  bc1nez $f0, 4
{                   "bc2eqz CASH , NUM",{0x49200000,0x001FFFFF}}, // 01001001001xxxxxxxxxxxxxxxxxxxxx  bc2eqz $0, 4
{                   "bc2nez CASH , NUM",{0x49A00000,0x001FFFFF}}, // 01001001101xxxxxxxxxxxxxxxxxxxxx  bc2nez $0, 4
{           "bclr.b WREG , WREG , WREG",{0x7980000D,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx001101  bclr.b $w0, $w0, $w0
{           "bclr.d WREG , WREG , WREG",{0x79E0000D,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx001101  bclr.d $w0, $w0, $w0
{           "bclr.h WREG , WREG , WREG",{0x79A0000D,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx001101  bclr.h $w0, $w0, $w0
{           "bclr.w WREG , WREG , WREG",{0x79C0000D,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx001101  bclr.w $w0, $w0, $w0
{           "bclri.b WREG , WREG , NUM",{0x79F00009,0x0007FFC0}}, // 0111100111110xxxxxxxxxxxxx001001  bclri.b $w0, $w0, 0
{           "bclri.d WREG , WREG , NUM",{0x79800009,0x003FFFC0}}, // 0111100110xxxxxxxxxxxxxxxx001001  bclri.d $w0, $w0, 0
{           "bclri.h WREG , WREG , NUM",{0x79E00009,0x000FFFC0}}, // 011110011110xxxxxxxxxxxxxx001001  bclri.h $w0, $w0, 0
{           "bclri.w WREG , WREG , NUM",{0x79C00009,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx001001  bclri.w $w0, $w0, 0
{             "beq GPREG , GPREG , NUM",{0x10010000,0x03FFFFFF}}, // 000100xxxxxxxxxxxxxxxxxxxxxxxxxx  beq $zero, $at, 4
{            "beqc GPREG , GPREG , NUM",{0x21090000,0x03FFFFFF}}, // 001000xxxxxxxxxxxxxxxxxxxxxxxxxx  beqc $t0, $t1, 0
{            "beql GPREG , GPREG , NUM",{0x51000000,0x03FFFFFF}}, // 010100xxxxxxxxxxxxxxxxxxxxxxxxxx  beql $t0, $zero, 4
{                    "beqz GPREG , NUM",{0x11000000,0x03E0FFFF}}, // 000100xxxxx00000xxxxxxxxxxxxxxxx  beqz $t0, 4
{                 "beqzalc GPREG , NUM",{0x20010000,0x001FFFFF}}, // 00100000000xxxxxxxxxxxxxxxxxxxxx  beqzalc $at, 0
{                   "beqzc GPREG , NUM",{0xD9000000,0x03FFFFFF}}, // 110110xxxxxxxxxxxxxxxxxxxxxxxxxx  beqzc $t0, 0
{                   "beqzl GPREG , NUM",{0x50000000,0x0000FFFF}}, // 0101000000000000xxxxxxxxxxxxxxxx  beqzl $zero, 4
{            "bgec GPREG , GPREG , NUM",{0x59010000,0x03FFFFFF}}, // 010110xxxxxxxxxxxxxxxxxxxxxxxxxx  bgec $t0, $at, 0
{           "bgeuc GPREG , GPREG , NUM",{0x19010000,0x03FFFFFF}}, // 000110xxxxxxxxxxxxxxxxxxxxxxxxxx  bgeuc $t0, $at, 0
{                    "bgez GPREG , NUM",{0x04010000,0x03E0FFFF}}, // 000001xxxxx00001xxxxxxxxxxxxxxxx  bgez $zero, 4
{                 "bgezalc GPREG , NUM",{0x19080000,0x03FFFFFF}}, // 000110xxxxxxxxxxxxxxxxxxxxxxxxxx  bgezalc $t0, 0
{                 "bgezall GPREG , NUM",{0x04130000,0x03E0FFFF}}, // 000001xxxxx10011xxxxxxxxxxxxxxxx  bgezall $zero, 4
{                   "bgezc GPREG , NUM",{0x59080000,0x03FFFFFF}}, // 010110xxxxxxxxxxxxxxxxxxxxxxxxxx  bgezc $t0, 0
{                   "bgezl GPREG , NUM",{0x04030000,0x03E0FFFF}}, // 000001xxxxx00011xxxxxxxxxxxxxxxx  bgezl $zero, 4
{                    "bgtz GPREG , NUM",{0x1C000000,0x03E0FFFF}}, // 000111xxxxx00000xxxxxxxxxxxxxxxx  bgtz $zero, 0
{                 "bgtzalc GPREG , NUM",{0x1C010000,0x001FFFFF}}, // 00011100000xxxxxxxxxxxxxxxxxxxxx  bgtzalc $at, 0
{                   "bgtzc GPREG , NUM",{0x5C010000,0x001FFFFF}}, // 01011100000xxxxxxxxxxxxxxxxxxxxx  bgtzc $at, 0
{                   "bgtzl GPREG , NUM",{0x5C000000,0x03E0FFFF}}, // 010111xxxxx00000xxxxxxxxxxxxxxxx  bgtzl $zero, 4
{          "binsl.b WREG , WREG , WREG",{0x7B00000D,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx001101  binsl.b $w0, $w0, $w0
{          "binsl.d WREG , WREG , WREG",{0x7B60000D,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx001101  binsl.d $w0, $w0, $w0
{          "binsl.h WREG , WREG , WREG",{0x7B20000D,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx001101  binsl.h $w0, $w0, $w0
{          "binsl.w WREG , WREG , WREG",{0x7B40000D,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx001101  binsl.w $w0, $w0, $w0
{          "binsli.b WREG , WREG , NUM",{0x7B700009,0x0007FFC0}}, // 0111101101110xxxxxxxxxxxxx001001  binsli.b $w0, $w0, 0
{          "binsli.d WREG , WREG , NUM",{0x7B000009,0x003FFFC0}}, // 0111101100xxxxxxxxxxxxxxxx001001  binsli.d $w0, $w0, 0
{          "binsli.h WREG , WREG , NUM",{0x7B600009,0x000FFFC0}}, // 011110110110xxxxxxxxxxxxxx001001  binsli.h $w0, $w0, 0
{          "binsli.w WREG , WREG , NUM",{0x7B400009,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx001001  binsli.w $w0, $w0, 0
{          "binsr.b WREG , WREG , WREG",{0x7B80000D,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx001101  binsr.b $w0, $w0, $w0
{          "binsr.d WREG , WREG , WREG",{0x7BE0000D,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx001101  binsr.d $w0, $w0, $w0
{          "binsr.h WREG , WREG , WREG",{0x7BA0000D,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx001101  binsr.h $w0, $w0, $w0
{          "binsr.w WREG , WREG , WREG",{0x7BC0000D,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx001101  binsr.w $w0, $w0, $w0
{          "binsri.b WREG , WREG , NUM",{0x7BF00009,0x0007FFC0}}, // 0111101111110xxxxxxxxxxxxx001001  binsri.b $w0, $w0, 0
{          "binsri.d WREG , WREG , NUM",{0x7B800009,0x003FFFC0}}, // 0111101110xxxxxxxxxxxxxxxx001001  binsri.d $w0, $w0, 0
{          "binsri.h WREG , WREG , NUM",{0x7BE00009,0x000FFFC0}}, // 011110111110xxxxxxxxxxxxxx001001  binsri.h $w0, $w0, 0
{          "binsri.w WREG , WREG , NUM",{0x7BC00009,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx001001  binsri.w $w0, $w0, 0
{                "bitrev GPREG , GPREG",{0x7C0006D2,0x001FF800}}, // 01111100000xxxxxxxxxx11011010010  bitrev $zero, $zero
{               "bitswap GPREG , GPREG",{0x7C000020,0x001FF800}}, // 01111100000xxxxxxxxxx00000100000  bitswap $zero, $zero
{                    "blez GPREG , NUM",{0x18000000,0x03E0FFFF}}, // 000110xxxxx00000xxxxxxxxxxxxxxxx  blez $zero, 4
{                 "blezalc GPREG , NUM",{0x18010000,0x001FFFFF}}, // 00011000000xxxxxxxxxxxxxxxxxxxxx  blezalc $at, 0
{                   "blezc GPREG , NUM",{0x58010000,0x001FFFFF}}, // 01011000000xxxxxxxxxxxxxxxxxxxxx  blezc $at, 0
{                   "blezl GPREG , NUM",{0x58000000,0x03E0FFFF}}, // 010110xxxxx00000xxxxxxxxxxxxxxxx  blezl $zero, 4
{            "bltc GPREG , GPREG , NUM",{0x5D010000,0x03FFFFFF}}, // 010111xxxxxxxxxxxxxxxxxxxxxxxxxx  bltc $t0, $at, 0
{           "bltuc GPREG , GPREG , NUM",{0x1D010000,0x03FFFFFF}}, // 000111xxxxxxxxxxxxxxxxxxxxxxxxxx  bltuc $t0, $at, 0
{                    "bltz GPREG , NUM",{0x04000000,0x03E0FFFF}}, // 000001xxxxx00000xxxxxxxxxxxxxxxx  bltz $zero, 4
{                 "bltzalc GPREG , NUM",{0x1D080000,0x03FFFFFF}}, // 000111xxxxxxxxxxxxxxxxxxxxxxxxxx  bltzalc $t0, 0
{                 "bltzall GPREG , NUM",{0x04120000,0x03E0FFFF}}, // 000001xxxxx10010xxxxxxxxxxxxxxxx  bltzall $zero, 4
{                   "bltzc GPREG , NUM",{0x5D080000,0x03FFFFFF}}, // 010111xxxxxxxxxxxxxxxxxxxxxxxxxx  bltzc $t0, 0
{                   "bltzl GPREG , NUM",{0x04020000,0x03E0FFFF}}, // 000001xxxxx00010xxxxxxxxxxxxxxxx  bltzl $zero, 4
{           "bmnz.v WREG , WREG , WREG",{0x7880001E,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx011110  bmnz.v $w0, $w0, $w0
{           "bmnzi.b WREG , WREG , NUM",{0x78000001,0x00FFFFC0}}, // 01111000xxxxxxxxxxxxxxxxxx000001  bmnzi.b $w0, $w0, 0
{            "bmz.v WREG , WREG , WREG",{0x78A0001E,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx011110  bmz.v $w0, $w0, $w0
{            "bmzi.b WREG , WREG , NUM",{0x79000001,0x00FFFFC0}}, // 01111001xxxxxxxxxxxxxxxxxx000001  bmzi.b $w0, $w0, 0
{             "bne GPREG , GPREG , NUM",{0x14010000,0x03FFFFFF}}, // 000101xxxxxxxxxxxxxxxxxxxxxxxxxx  bne $zero, $at, 4
{            "bnec GPREG , GPREG , NUM",{0x61090000,0x03FFFFFF}}, // 011000xxxxxxxxxxxxxxxxxxxxxxxxxx  bnec $t0, $t1, 0
{           "bneg.b WREG , WREG , WREG",{0x7A80000D,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx001101  bneg.b $w0, $w0, $w0
{           "bneg.d WREG , WREG , WREG",{0x7AE0000D,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx001101  bneg.d $w0, $w0, $w0
{           "bneg.h WREG , WREG , WREG",{0x7AA0000D,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx001101  bneg.h $w0, $w0, $w0
{           "bneg.w WREG , WREG , WREG",{0x7AC0000D,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx001101  bneg.w $w0, $w0, $w0
{           "bnegi.b WREG , WREG , NUM",{0x7AF00009,0x0007FFC0}}, // 0111101011110xxxxxxxxxxxxx001001  bnegi.b $w0, $w0, 0
{           "bnegi.d WREG , WREG , NUM",{0x7A800009,0x003FFFC0}}, // 0111101010xxxxxxxxxxxxxxxx001001  bnegi.d $w0, $w0, 0
{           "bnegi.h WREG , WREG , NUM",{0x7AE00009,0x000FFFC0}}, // 011110101110xxxxxxxxxxxxxx001001  bnegi.h $w0, $w0, 0
{           "bnegi.w WREG , WREG , NUM",{0x7AC00009,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx001001  bnegi.w $w0, $w0, 0
{            "bnel GPREG , GPREG , NUM",{0x54010000,0x03FFFFFF}}, // 010101xxxxxxxxxxxxxxxxxxxxxxxxxx  bnel $zero, $at, 4
{                    "bnez GPREG , NUM",{0x14000000,0x03E0FFFF}}, // 000101xxxxx00000xxxxxxxxxxxxxxxx  bnez $zero, 4
{                 "bnezalc GPREG , NUM",{0x60010000,0x001FFFFF}}, // 01100000000xxxxxxxxxxxxxxxxxxxxx  bnezalc $at, 0
{                   "bnezc GPREG , NUM",{0xF9000000,0x03FFFFFF}}, // 111110xxxxxxxxxxxxxxxxxxxxxxxxxx  bnezc $t0, 0
{                   "bnezl GPREG , NUM",{0x54000000,0x03E0FFFF}}, // 010101xxxxx00000xxxxxxxxxxxxxxxx  bnezl $zero, 4
{            "bnvc GPREG , GPREG , NUM",{0x60000000,0x03FFFFFF}}, // 011000xxxxxxxxxxxxxxxxxxxxxxxxxx  bnvc $zero, $zero, 0
{                    "bnz.b WREG , NUM",{0x47800000,0x001FFFFF}}, // 01000111100xxxxxxxxxxxxxxxxxxxxx  bnz.b $w0, 4
{                    "bnz.d WREG , NUM",{0x47E00000,0x001FFFFF}}, // 01000111111xxxxxxxxxxxxxxxxxxxxx  bnz.d $w0, 4
{                    "bnz.h WREG , NUM",{0x47A00000,0x001FFFFF}}, // 01000111101xxxxxxxxxxxxxxxxxxxxx  bnz.h $w0, 4
{                    "bnz.v WREG , NUM",{0x45E00000,0x001FFFFF}}, // 01000101111xxxxxxxxxxxxxxxxxxxxx  bnz.v $w0, 4
{                    "bnz.w WREG , NUM",{0x47C00000,0x001FFFFF}}, // 01000111110xxxxxxxxxxxxxxxxxxxxx  bnz.w $w0, 4
{            "bovc GPREG , GPREG , NUM",{0x20000000,0x03FFFFFF}}, // 001000xxxxxxxxxxxxxxxxxxxxxxxxxx  bovc $zero, $zero, 0
{                        "bposge32 NUM",{0x041C0000,0x0000FFFF}}, // 0000010000011100xxxxxxxxxxxxxxxx  bposge32 4
{                               "break",{0x0000000D,0x00000000}}, // 00000000000000000000000000001101  break
{                           "break NUM",{0x0200000D,0x03FF0000}}, // 000000xxxxxxxxxx0000000000001101  break 0x200
{                     "break NUM , NUM",{0x0000010D,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx001101  break 0, 4
{           "bsel.v WREG , WREG , WREG",{0x78C0001E,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx011110  bsel.v $w0, $w0, $w0
{           "bseli.b WREG , WREG , NUM",{0x7A000001,0x00FFFFC0}}, // 01111010xxxxxxxxxxxxxxxxxx000001  bseli.b $w0, $w0, 0
{           "bset.b WREG , WREG , WREG",{0x7A00000D,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx001101  bset.b $w0, $w0, $w0
{           "bset.d WREG , WREG , WREG",{0x7A60000D,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx001101  bset.d $w0, $w0, $w0
{           "bset.h WREG , WREG , WREG",{0x7A20000D,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx001101  bset.h $w0, $w0, $w0
{           "bset.w WREG , WREG , WREG",{0x7A40000D,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx001101  bset.w $w0, $w0, $w0
{           "bseti.b WREG , WREG , NUM",{0x7A700009,0x0007FFC0}}, // 0111101001110xxxxxxxxxxxxx001001  bseti.b $w0, $w0, 0
{           "bseti.d WREG , WREG , NUM",{0x7A000009,0x003FFFC0}}, // 0111101000xxxxxxxxxxxxxxxx001001  bseti.d $w0, $w0, 0
{           "bseti.h WREG , WREG , NUM",{0x7A600009,0x000FFFC0}}, // 011110100110xxxxxxxxxxxxxx001001  bseti.h $w0, $w0, 0
{           "bseti.w WREG , WREG , NUM",{0x7A400009,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx001001  bseti.w $w0, $w0, 0
{                     "bz.b WREG , NUM",{0x47000000,0x001FFFFF}}, // 01000111000xxxxxxxxxxxxxxxxxxxxx  bz.b $w0, 4
{                     "bz.d WREG , NUM",{0x47600000,0x001FFFFF}}, // 01000111011xxxxxxxxxxxxxxxxxxxxx  bz.d $w0, 4
{                     "bz.h WREG , NUM",{0x47200000,0x001FFFFF}}, // 01000111001xxxxxxxxxxxxxxxxxxxxx  bz.h $w0, 4
{                     "bz.v WREG , NUM",{0x45600000,0x001FFFFF}}, // 01000101011xxxxxxxxxxxxxxxxxxxxx  bz.v $w0, 4
{                     "bz.w WREG , NUM",{0x47400000,0x001FFFFF}}, // 01000111010xxxxxxxxxxxxxxxxxxxxx  bz.w $w0, 4
{                         "cache , ( )",{0x7C000025,0x00000000}}, // 01111100000000000000000000100101  cache 0x450, ()
{                     "cache , ( NUM )",{0x7E000025,0x03E0FF80}}, // 011111xxxxx00000xxxxxxxxx0100101  cache 0x450, (0x100000)
{                     "cache , NUM ( )",{0x7C010025,0x001F0000}}, // 01111100000xxxxx0000000000100101  cache 0x450, 1()
{                 "cache , NUM ( NUM )",{0x7E010025,0x03FFFF80}}, // 011111xxxxxxxxxxxxxxxxxxx0100101  cache 0x450, 1(0x100000)
{                "ceil.w.d FREG , FREG",{0x4620000E,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx001110  ceil.w.d $f0, $f0
{                "ceil.w.s FREG , FREG",{0x4600000E,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx001110  ceil.w.s $f0, $f0
{            "ceq.b WREG , WREG , WREG",{0x7800000F,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx001111  ceq.b $w0, $w0, $w0
{            "ceq.d WREG , WREG , WREG",{0x7860000F,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx001111  ceq.d $w0, $w0, $w0
{            "ceq.h WREG , WREG , WREG",{0x7820000F,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx001111  ceq.h $w0, $w0, $w0
{            "ceq.w WREG , WREG , WREG",{0x7840000F,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx001111  ceq.w $w0, $w0, $w0
{            "ceqi.b WREG , WREG , NUM",{0x78000007,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx000111  ceqi.b $w0, $w0, 0
{            "ceqi.d WREG , WREG , NUM",{0x78600007,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx000111  ceqi.d $w0, $w0, 0
{            "ceqi.h WREG , WREG , NUM",{0x78200007,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx000111  ceqi.h $w0, $w0, 0
{            "ceqi.w WREG , WREG , NUM",{0x78400007,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx000111  ceqi.w $w0, $w0, 0
{                   "cfc1 GPREG , CASH",{0x44400000,0x001FF800}}, // 01000100010xxxxxxxxxx00000000000  cfc1 $zero, $0
{                 "cfcmsa GPREG , CASH",{0x787E0019,0x00003FC0}}, // 011110000111111000xxxxxxxx011001  cfcmsa $zero, $0
{                 "class.d FREG , FREG",{0x4620001B,0x0000FFC0}}, // 0100011000100000xxxxxxxxxx011011  class.d $f0, $f0
{                 "class.s FREG , FREG",{0x4600001B,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx011011  class.s $f0, $f0
{          "cle_s.b WREG , WREG , WREG",{0x7A00000F,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx001111  cle_s.b $w0, $w0, $w0
{          "cle_s.d WREG , WREG , WREG",{0x7A60000F,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx001111  cle_s.d $w0, $w0, $w0
{          "cle_s.h WREG , WREG , WREG",{0x7A20000F,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx001111  cle_s.h $w0, $w0, $w0
{          "cle_s.w WREG , WREG , WREG",{0x7A40000F,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx001111  cle_s.w $w0, $w0, $w0
{          "cle_u.b WREG , WREG , WREG",{0x7A80000F,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx001111  cle_u.b $w0, $w0, $w0
{          "cle_u.d WREG , WREG , WREG",{0x7AE0000F,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx001111  cle_u.d $w0, $w0, $w0
{          "cle_u.h WREG , WREG , WREG",{0x7AA0000F,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx001111  cle_u.h $w0, $w0, $w0
{          "cle_u.w WREG , WREG , WREG",{0x7AC0000F,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx001111  cle_u.w $w0, $w0, $w0
{          "clei_s.b WREG , WREG , NUM",{0x7A000007,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx000111  clei_s.b $w0, $w0, 0
{          "clei_s.d WREG , WREG , NUM",{0x7A600007,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx000111  clei_s.d $w0, $w0, 0
{          "clei_s.h WREG , WREG , NUM",{0x7A200007,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx000111  clei_s.h $w0, $w0, 0
{          "clei_s.w WREG , WREG , NUM",{0x7A400007,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx000111  clei_s.w $w0, $w0, 0
{          "clei_u.b WREG , WREG , NUM",{0x7A800007,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx000111  clei_u.b $w0, $w0, 0
{          "clei_u.d WREG , WREG , NUM",{0x7AE00007,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx000111  clei_u.d $w0, $w0, 0
{          "clei_u.h WREG , WREG , NUM",{0x7AA00007,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx000111  clei_u.h $w0, $w0, 0
{          "clei_u.w WREG , WREG , NUM",{0x7AC00007,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx000111  clei_u.w $w0, $w0, 0
{                   "clo GPREG , GPREG",{0x00000051,0x03E0F800}}, // 000000xxxxx00000xxxxx00001010001  clo $zero, $zero
{          "clt_s.b WREG , WREG , WREG",{0x7900000F,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx001111  clt_s.b $w0, $w0, $w0
{          "clt_s.d WREG , WREG , WREG",{0x7960000F,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx001111  clt_s.d $w0, $w0, $w0
{          "clt_s.h WREG , WREG , WREG",{0x7920000F,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx001111  clt_s.h $w0, $w0, $w0
{          "clt_s.w WREG , WREG , WREG",{0x7940000F,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx001111  clt_s.w $w0, $w0, $w0
{          "clt_u.b WREG , WREG , WREG",{0x7980000F,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx001111  clt_u.b $w0, $w0, $w0
{          "clt_u.d WREG , WREG , WREG",{0x79E0000F,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx001111  clt_u.d $w0, $w0, $w0
{          "clt_u.h WREG , WREG , WREG",{0x79A0000F,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx001111  clt_u.h $w0, $w0, $w0
{          "clt_u.w WREG , WREG , WREG",{0x79C0000F,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx001111  clt_u.w $w0, $w0, $w0
{          "clti_s.b WREG , WREG , NUM",{0x79000007,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx000111  clti_s.b $w0, $w0, 0
{          "clti_s.d WREG , WREG , NUM",{0x79600007,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx000111  clti_s.d $w0, $w0, 0
{          "clti_s.h WREG , WREG , NUM",{0x79200007,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx000111  clti_s.h $w0, $w0, 0
{          "clti_s.w WREG , WREG , NUM",{0x79400007,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx000111  clti_s.w $w0, $w0, 0
{          "clti_u.b WREG , WREG , NUM",{0x79800007,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx000111  clti_u.b $w0, $w0, 0
{          "clti_u.d WREG , WREG , NUM",{0x79E00007,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx000111  clti_u.d $w0, $w0, 0
{          "clti_u.h WREG , WREG , NUM",{0x79A00007,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx000111  clti_u.h $w0, $w0, 0
{          "clti_u.w WREG , WREG , NUM",{0x79C00007,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx000111  clti_u.w $w0, $w0, 0
{                   "clz GPREG , GPREG",{0x00000050,0x03E0F800}}, // 000000xxxxx00000xxxxx00001010000  clz $zero, $zero
{         "cmp.af.d FREG , FREG , FREG",{0x46A00000,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000000  cmp.af.d $f0, $f0, $f0
{         "cmp.af.s FREG , FREG , FREG",{0x46800000,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000000  cmp.af.s $f0, $f0, $f0
{         "cmp.eq.d FREG , FREG , FREG",{0x46A00002,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000010  cmp.eq.d $f0, $f0, $f0
{             "cmp.eq.ph GPREG , GPREG",{0x7C000211,0x03FF0000}}, // 011111xxxxxxxxxx0000001000010001  cmp.eq.ph $zero, $zero
{         "cmp.eq.s FREG , FREG , FREG",{0x46800002,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000010  cmp.eq.s $f0, $f0, $f0
{         "cmp.le.d FREG , FREG , FREG",{0x46A00006,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000110  cmp.le.d $f0, $f0, $f0
{             "cmp.le.ph GPREG , GPREG",{0x7C000291,0x03FF0000}}, // 011111xxxxxxxxxx0000001010010001  cmp.le.ph $zero, $zero
{         "cmp.le.s FREG , FREG , FREG",{0x46800006,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000110  cmp.le.s $f0, $f0, $f0
{         "cmp.lt.d FREG , FREG , FREG",{0x46A00004,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000100  cmp.lt.d $f0, $f0, $f0
{             "cmp.lt.ph GPREG , GPREG",{0x7C000251,0x03FF0000}}, // 011111xxxxxxxxxx0000001001010001  cmp.lt.ph $zero, $zero
{         "cmp.lt.s FREG , FREG , FREG",{0x46800004,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000100  cmp.lt.s $f0, $f0, $f0
{        "cmp.saf.d FREG , FREG , FREG",{0x46A00008,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001000  cmp.saf.d $f0, $f0, $f0
{        "cmp.saf.s FREG , FREG , FREG",{0x46800008,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001000  cmp.saf.s $f0, $f0, $f0
{        "cmp.seq.d FREG , FREG , FREG",{0x46A0000A,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001010  cmp.seq.d $f0, $f0, $f0
{        "cmp.seq.s FREG , FREG , FREG",{0x4680000A,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001010  cmp.seq.s $f0, $f0, $f0
{        "cmp.sle.d FREG , FREG , FREG",{0x46A0000E,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001110  cmp.sle.d $f0, $f0, $f0
{        "cmp.sle.s FREG , FREG , FREG",{0x4680000E,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001110  cmp.sle.s $f0, $f0, $f0
{        "cmp.slt.d FREG , FREG , FREG",{0x46A0000C,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001100  cmp.slt.d $f0, $f0, $f0
{        "cmp.slt.s FREG , FREG , FREG",{0x4680000C,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001100  cmp.slt.s $f0, $f0, $f0
{       "cmp.sueq.d FREG , FREG , FREG",{0x46A0000B,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001011  cmp.sueq.d $f0, $f0, $f0
{       "cmp.sueq.s FREG , FREG , FREG",{0x4680000B,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001011  cmp.sueq.s $f0, $f0, $f0
{       "cmp.sule.d FREG , FREG , FREG",{0x46A0000F,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001111  cmp.sule.d $f0, $f0, $f0
{       "cmp.sule.s FREG , FREG , FREG",{0x4680000F,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001111  cmp.sule.s $f0, $f0, $f0
{       "cmp.sult.d FREG , FREG , FREG",{0x46A0000D,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001101  cmp.sult.d $f0, $f0, $f0
{       "cmp.sult.s FREG , FREG , FREG",{0x4680000D,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001101  cmp.sult.s $f0, $f0, $f0
{        "cmp.sun.d FREG , FREG , FREG",{0x46A00009,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx001001  cmp.sun.d $f0, $f0, $f0
{        "cmp.sun.s FREG , FREG , FREG",{0x46800009,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx001001  cmp.sun.s $f0, $f0, $f0
{        "cmp.ueq.d FREG , FREG , FREG",{0x46A00003,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000011  cmp.ueq.d $f0, $f0, $f0
{        "cmp.ueq.s FREG , FREG , FREG",{0x46800003,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000011  cmp.ueq.s $f0, $f0, $f0
{        "cmp.ule.d FREG , FREG , FREG",{0x46A00007,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000111  cmp.ule.d $f0, $f0, $f0
{        "cmp.ule.s FREG , FREG , FREG",{0x46800007,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000111  cmp.ule.s $f0, $f0, $f0
{        "cmp.ult.d FREG , FREG , FREG",{0x46A00005,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000101  cmp.ult.d $f0, $f0, $f0
{        "cmp.ult.s FREG , FREG , FREG",{0x46800005,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000101  cmp.ult.s $f0, $f0, $f0
{         "cmp.un.d FREG , FREG , FREG",{0x46A00001,0x001FFFC0}}, // 01000110101xxxxxxxxxxxxxxx000001  cmp.un.d $f0, $f0, $f0
{         "cmp.un.s FREG , FREG , FREG",{0x46800001,0x001FFFC0}}, // 01000110100xxxxxxxxxxxxxxx000001  cmp.un.s $f0, $f0, $f0
{  "cmpgdu.eq.qb GPREG , GPREG , GPREG",{0x7C000611,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11000010001  cmpgdu.eq.qb $zero, $zero, $zero
{  "cmpgdu.le.qb GPREG , GPREG , GPREG",{0x7C000691,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11010010001  cmpgdu.le.qb $zero, $zero, $zero
{  "cmpgdu.lt.qb GPREG , GPREG , GPREG",{0x7C000651,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11001010001  cmpgdu.lt.qb $zero, $zero, $zero
{   "cmpgu.eq.qb GPREG , GPREG , GPREG",{0x7C000111,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00100010001  cmpgu.eq.qb $zero, $zero, $zero
{   "cmpgu.le.qb GPREG , GPREG , GPREG",{0x7C000191,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00110010001  cmpgu.le.qb $zero, $zero, $zero
{   "cmpgu.lt.qb GPREG , GPREG , GPREG",{0x7C000151,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00101010001  cmpgu.lt.qb $zero, $zero, $zero
{            "cmpu.eq.qb GPREG , GPREG",{0x7C000011,0x03FF0000}}, // 011111xxxxxxxxxx0000000000010001  cmpu.eq.qb $zero, $zero
{            "cmpu.le.qb GPREG , GPREG",{0x7C000091,0x03FF0000}}, // 011111xxxxxxxxxx0000000010010001  cmpu.le.qb $zero, $zero
{            "cmpu.lt.qb GPREG , GPREG",{0x7C000051,0x03FF0000}}, // 011111xxxxxxxxxx0000000001010001  cmpu.lt.qb $zero, $zero
{       "copy_s.b GPREG , WREG [ NUM ]",{0x78800019,0x000FFFC0}}, // 011110001000xxxxxxxxxxxxxx011001  copy_s.b $zero, $w0[0]
{       "copy_s.d GPREG , WREG [ NUM ]",{0x78B80019,0x0001FFC0}}, // 011110001011100xxxxxxxxxxx011001  copy_s.d $zero, $w0[0]
{       "copy_s.h GPREG , WREG [ NUM ]",{0x78A00019,0x0007FFC0}}, // 0111100010100xxxxxxxxxxxxx011001  copy_s.h $zero, $w0[0]
{       "copy_s.w GPREG , WREG [ NUM ]",{0x78B00019,0x0003FFC0}}, // 01111000101100xxxxxxxxxxxx011001  copy_s.w $zero, $w0[0]
{       "copy_u.b GPREG , WREG [ NUM ]",{0x78C00019,0x000FFFC0}}, // 011110001100xxxxxxxxxxxxxx011001  copy_u.b $zero, $w0[0]
{       "copy_u.d GPREG , WREG [ NUM ]",{0x78F80019,0x0001FFC0}}, // 011110001111100xxxxxxxxxxx011001  copy_u.d $zero, $w0[0]
{       "copy_u.h GPREG , WREG [ NUM ]",{0x78E00019,0x0007FFC0}}, // 0111100011100xxxxxxxxxxxxx011001  copy_u.h $zero, $w0[0]
{       "copy_u.w GPREG , WREG [ NUM ]",{0x78F00019,0x0003FFC0}}, // 01111000111100xxxxxxxxxxxx011001  copy_u.w $zero, $w0[0]
{                   "ctc1 GPREG , CASH",{0x44C00000,0x001FF800}}, // 01000100110xxxxxxxxxx00000000000  ctc1 $zero, $0
{                 "ctcmsa CASH , GPREG",{0x783E0019,0x0000F9C0}}, // 0111100000111110xxxxx00xxx011001  ctcmsa $0, $zero
{                 "cvt.d.s FREG , FREG",{0x46000021,0x0000FF80}}, // 0100011000000000xxxxxxxxx0100001  cvt.d.s $f0, $f0
{                 "cvt.d.w FREG , FREG",{0x46800021,0x0000FF80}}, // 0100011010000000xxxxxxxxx0100001  cvt.d.w $f0, $f0
{                 "cvt.l.d FREG , FREG",{0x46200025,0x0000FFC0}}, // 0100011000100000xxxxxxxxxx100101  cvt.l.d $f0, $f0
{                 "cvt.l.s FREG , FREG",{0x46000025,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx100101  cvt.l.s $f0, $f0
{                 "cvt.s.d FREG , FREG",{0x46200020,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx100000  cvt.s.d $f0, $f0
{                 "cvt.s.w FREG , FREG",{0x46800020,0x0000FFC0}}, // 0100011010000000xxxxxxxxxx100000  cvt.s.w $f0, $f0
{                 "cvt.w.d FREG , FREG",{0x46200024,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx100100  cvt.w.d $f0, $f0
{                 "cvt.w.s FREG , FREG",{0x46000024,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx100100  cvt.w.s $f0, $f0
{                               "deret",{0x4200001F,0x00000000}}, // 01000010000000000000000000011111  deret
{                                  "di",{0x41606000,0x00000000}}, // 01000001011000000110000000000000  di
{                            "di GPREG",{0x41616000,0x001F0000}}, // 01000001011xxxxx0110000000000000  di $at
{           "div GPREG , GPREG , GPREG",{0x0000009A,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00010011010  div $zero, $zero, $zero
{            "div.d FREG , FREG , FREG",{0x46200003,0x001EF780}}, // 01000110001xxxx0xxxx0xxxx0000011  div.d $f0, $f0, $f0
{            "div.s FREG , FREG , FREG",{0x46000003,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx000011  div.s $f0, $f0, $f0
{          "div_s.b WREG , WREG , WREG",{0x7A000012,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx010010  div_s.b $w0, $w0, $w0
{          "div_s.d WREG , WREG , WREG",{0x7A600012,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010010  div_s.d $w0, $w0, $w0
{          "div_s.h WREG , WREG , WREG",{0x7A200012,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010010  div_s.h $w0, $w0, $w0
{          "div_s.w WREG , WREG , WREG",{0x7A400012,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010010  div_s.w $w0, $w0, $w0
{          "div_u.b WREG , WREG , WREG",{0x7A800012,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx010010  div_u.b $w0, $w0, $w0
{          "div_u.d WREG , WREG , WREG",{0x7AE00012,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010010  div_u.d $w0, $w0, $w0
{          "div_u.h WREG , WREG , WREG",{0x7AA00012,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010010  div_u.h $w0, $w0, $w0
{          "div_u.w WREG , WREG , WREG",{0x7AC00012,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010010  div_u.w $w0, $w0, $w0
{          "divu GPREG , GPREG , GPREG",{0x0000009B,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00010011011  divu $zero, $zero, $zero
{    "dlsa GPREG , GPREG , GPREG , NUM",{0x00000015,0x03FFF8C0}}, // 000000xxxxxxxxxxxxxxx000xx010101  dlsa $zero, $zero, $zero, 1
{                  "dmfc1 GPREG , FREG",{0x44200000,0x001FF800}}, // 01000100001xxxxxxxxxx00000000000  dmfc1 $zero, $f0
{                  "dmtc1 GPREG , FREG",{0x44A00000,0x001FF800}}, // 01000100101xxxxxxxxxx00000000000  dmtc1 $zero, $f0
{         "dotp_s.d WREG , WREG , WREG",{0x78600013,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010011  dotp_s.d $w0, $w0, $w0
{         "dotp_s.h WREG , WREG , WREG",{0x78200013,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010011  dotp_s.h $w0, $w0, $w0
{         "dotp_s.w WREG , WREG , WREG",{0x78400013,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010011  dotp_s.w $w0, $w0, $w0
{         "dotp_u.d WREG , WREG , WREG",{0x78E00013,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010011  dotp_u.d $w0, $w0, $w0
{         "dotp_u.h WREG , WREG , WREG",{0x78A00013,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010011  dotp_u.h $w0, $w0, $w0
{         "dotp_u.w WREG , WREG , WREG",{0x78C00013,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010011  dotp_u.w $w0, $w0, $w0
{      "dpa.w.ph ACREG , GPREG , GPREG",{0x7C000030,0x03FF1800}}, // 011111xxxxxxxxxx000xx00000110000  dpa.w.ph $ac0, $zero, $zero
{        "dpadd_s.d WREG , WREG , WREG",{0x79600013,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010011  dpadd_s.d $w0, $w0, $w0
{        "dpadd_s.h WREG , WREG , WREG",{0x79200013,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010011  dpadd_s.h $w0, $w0, $w0
{        "dpadd_s.w WREG , WREG , WREG",{0x79400013,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010011  dpadd_s.w $w0, $w0, $w0
{        "dpadd_u.d WREG , WREG , WREG",{0x79E00013,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx010011  dpadd_u.d $w0, $w0, $w0
{        "dpadd_u.h WREG , WREG , WREG",{0x79A00013,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx010011  dpadd_u.h $w0, $w0, $w0
{        "dpadd_u.w WREG , WREG , WREG",{0x79C00013,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx010011  dpadd_u.w $w0, $w0, $w0
{   "dpaq_s.w.ph ACREG , GPREG , GPREG",{0x7C000130,0x03FF1800}}, // 011111xxxxxxxxxx000xx00100110000  dpaq_s.w.ph $ac0, $zero, $zero
{   "dpaq_sa.l.w ACREG , GPREG , GPREG",{0x7C000330,0x03FF1800}}, // 011111xxxxxxxxxx000xx01100110000  dpaq_sa.l.w $ac0, $zero, $zero
{  "dpaqx_s.w.ph ACREG , GPREG , GPREG",{0x7C000630,0x03FF1800}}, // 011111xxxxxxxxxx000xx11000110000  dpaqx_s.w.ph $ac0, $zero, $zero
{ "dpaqx_sa.w.ph ACREG , GPREG , GPREG",{0x7C0006B0,0x03FF1800}}, // 011111xxxxxxxxxx000xx11010110000  dpaqx_sa.w.ph $ac0, $zero, $zero
{    "dpau.h.qbl ACREG , GPREG , GPREG",{0x7C0000F0,0x03FF1800}}, // 011111xxxxxxxxxx000xx00011110000  dpau.h.qbl $ac0, $zero, $zero
{    "dpau.h.qbr ACREG , GPREG , GPREG",{0x7C0001F0,0x03FF1800}}, // 011111xxxxxxxxxx000xx00111110000  dpau.h.qbr $ac0, $zero, $zero
{     "dpax.w.ph ACREG , GPREG , GPREG",{0x7C000230,0x03FF1800}}, // 011111xxxxxxxxxx000xx01000110000  dpax.w.ph $ac0, $zero, $zero
{      "dps.w.ph ACREG , GPREG , GPREG",{0x7C000070,0x03FF1800}}, // 011111xxxxxxxxxx000xx00001110000  dps.w.ph $ac0, $zero, $zero
{   "dpsq_s.w.ph ACREG , GPREG , GPREG",{0x7C000170,0x03FF1800}}, // 011111xxxxxxxxxx000xx00101110000  dpsq_s.w.ph $ac0, $zero, $zero
{   "dpsq_sa.l.w ACREG , GPREG , GPREG",{0x7C000370,0x03FF1800}}, // 011111xxxxxxxxxx000xx01101110000  dpsq_sa.l.w $ac0, $zero, $zero
{  "dpsqx_s.w.ph ACREG , GPREG , GPREG",{0x7C000670,0x03FF1800}}, // 011111xxxxxxxxxx000xx11001110000  dpsqx_s.w.ph $ac0, $zero, $zero
{ "dpsqx_sa.w.ph ACREG , GPREG , GPREG",{0x7C0006F0,0x03FF1800}}, // 011111xxxxxxxxxx000xx11011110000  dpsqx_sa.w.ph $ac0, $zero, $zero
{    "dpsu.h.qbl ACREG , GPREG , GPREG",{0x7C0002F0,0x03FF1800}}, // 011111xxxxxxxxxx000xx01011110000  dpsu.h.qbl $ac0, $zero, $zero
{    "dpsu.h.qbr ACREG , GPREG , GPREG",{0x7C0003F0,0x03FF1800}}, // 011111xxxxxxxxxx000xx01111110000  dpsu.h.qbr $ac0, $zero, $zero
{        "dpsub_s.d WREG , WREG , WREG",{0x7A600013,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010011  dpsub_s.d $w0, $w0, $w0
{        "dpsub_s.h WREG , WREG , WREG",{0x7A200013,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010011  dpsub_s.h $w0, $w0, $w0
{        "dpsub_s.w WREG , WREG , WREG",{0x7A400013,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010011  dpsub_s.w $w0, $w0, $w0
{        "dpsub_u.d WREG , WREG , WREG",{0x7AE00013,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010011  dpsub_u.d $w0, $w0, $w0
{        "dpsub_u.h WREG , WREG , WREG",{0x7AA00013,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010011  dpsub_u.h $w0, $w0, $w0
{        "dpsub_u.w WREG , WREG , WREG",{0x7AC00013,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010011  dpsub_u.w $w0, $w0, $w0
{     "dpsx.w.ph ACREG , GPREG , GPREG",{0x7C000270,0x03FF1800}}, // 011111xxxxxxxxxx000xx01001110000  dpsx.w.ph $ac0, $zero, $zero
{                                 "ehb",{0x000000C0,0x00000000}}, // 00000000000000000000000011000000  ehb
{                                  "ei",{0x41606020,0x00000000}}, // 01000001011000000110000000100000  ei
{                            "ei GPREG",{0x41616020,0x001F0000}}, // 01000001011xxxxx0110000000100000  ei $at
{                                "eret",{0x42000018,0x00000000}}, // 01000010000000000000000000011000  eret
{       "ext GPREG , GPREG , NUM , NUM",{0x7C000000,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx000000  ext $zero, $zero, 0, 1
{            "extp GPREG , ACREG , NUM",{0x7C0000B8,0x03FF1800}}, // 011111xxxxxxxxxx000xx00010111000  extp $zero, $ac0, 0
{          "extpdp GPREG , ACREG , NUM",{0x7C0002B8,0x03FF1800}}, // 011111xxxxxxxxxx000xx01010111000  extpdp $zero, $ac0, 0
{       "extpdpv GPREG , ACREG , GPREG",{0x7C0002F8,0x03FF1800}}, // 011111xxxxxxxxxx000xx01011111000  extpdpv $zero, $ac0, $zero
{         "extpv GPREG , ACREG , GPREG",{0x7C0000F8,0x03FF1800}}, // 011111xxxxxxxxxx000xx00011111000  extpv $zero, $ac0, $zero
{          "extr.w GPREG , ACREG , NUM",{0x7C000038,0x03FF1800}}, // 011111xxxxxxxxxx000xx00000111000  extr.w $zero, $ac0, 0
{        "extr_r.w GPREG , ACREG , NUM",{0x7C000138,0x03FF1800}}, // 011111xxxxxxxxxx000xx00100111000  extr_r.w $zero, $ac0, 0
{       "extr_rs.w GPREG , ACREG , NUM",{0x7C0001B8,0x03FF1800}}, // 011111xxxxxxxxxx000xx00110111000  extr_rs.w $zero, $ac0, 0
{        "extr_s.h GPREG , ACREG , NUM",{0x7C0003B8,0x03FF1800}}, // 011111xxxxxxxxxx000xx01110111000  extr_s.h $zero, $ac0, 0
{       "extrv.w GPREG , ACREG , GPREG",{0x7C000078,0x03FF1800}}, // 011111xxxxxxxxxx000xx00001111000  extrv.w $zero, $ac0, $zero
{     "extrv_r.w GPREG , ACREG , GPREG",{0x7C000178,0x03FF1800}}, // 011111xxxxxxxxxx000xx00101111000  extrv_r.w $zero, $ac0, $zero
{    "extrv_rs.w GPREG , ACREG , GPREG",{0x7C0001F8,0x03FF1800}}, // 011111xxxxxxxxxx000xx00111111000  extrv_rs.w $zero, $ac0, $zero
{     "extrv_s.h GPREG , ACREG , GPREG",{0x7C0003F8,0x03FF1800}}, // 011111xxxxxxxxxx000xx01111111000  extrv_s.h $zero, $ac0, $zero
{           "fadd.d WREG , WREG , WREG",{0x7820001B,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx011011  fadd.d $w0, $w0, $w0
{           "fadd.w WREG , WREG , WREG",{0x7800001B,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx011011  fadd.w $w0, $w0, $w0
{           "fcaf.d WREG , WREG , WREG",{0x7820001A,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx011010  fcaf.d $w0, $w0, $w0
{           "fcaf.w WREG , WREG , WREG",{0x7800001A,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx011010  fcaf.w $w0, $w0, $w0
{           "fceq.d WREG , WREG , WREG",{0x78A0001A,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx011010  fceq.d $w0, $w0, $w0
{           "fceq.w WREG , WREG , WREG",{0x7880001A,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx011010  fceq.w $w0, $w0, $w0
{                "fclass.d WREG , WREG",{0x7B21001E,0x0000FFC0}}, // 0111101100100001xxxxxxxxxx011110  fclass.d $w0, $w0
{                "fclass.w WREG , WREG",{0x7B20001E,0x0000FFC0}}, // 0111101100100000xxxxxxxxxx011110  fclass.w $w0, $w0
{           "fcle.d WREG , WREG , WREG",{0x79A0001A,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx011010  fcle.d $w0, $w0, $w0
{           "fcle.w WREG , WREG , WREG",{0x7980001A,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx011010  fcle.w $w0, $w0, $w0
{           "fclt.d WREG , WREG , WREG",{0x7920001A,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx011010  fclt.d $w0, $w0, $w0
{           "fclt.w WREG , WREG , WREG",{0x7900001A,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx011010  fclt.w $w0, $w0, $w0
{           "fcne.d WREG , WREG , WREG",{0x78E0001C,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx011100  fcne.d $w0, $w0, $w0
{           "fcne.w WREG , WREG , WREG",{0x78C0001C,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx011100  fcne.w $w0, $w0, $w0
{           "fcor.d WREG , WREG , WREG",{0x7860001C,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx011100  fcor.d $w0, $w0, $w0
{           "fcor.w WREG , WREG , WREG",{0x7840001C,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx011100  fcor.w $w0, $w0, $w0
{          "fcueq.d WREG , WREG , WREG",{0x78E0001A,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx011010  fcueq.d $w0, $w0, $w0
{          "fcueq.w WREG , WREG , WREG",{0x78C0001A,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx011010  fcueq.w $w0, $w0, $w0
{          "fcule.d WREG , WREG , WREG",{0x79E0001A,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx011010  fcule.d $w0, $w0, $w0
{          "fcule.w WREG , WREG , WREG",{0x79C0001A,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx011010  fcule.w $w0, $w0, $w0
{          "fcult.d WREG , WREG , WREG",{0x7960001A,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx011010  fcult.d $w0, $w0, $w0
{          "fcult.w WREG , WREG , WREG",{0x7940001A,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx011010  fcult.w $w0, $w0, $w0
{           "fcun.d WREG , WREG , WREG",{0x7860001A,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx011010  fcun.d $w0, $w0, $w0
{           "fcun.w WREG , WREG , WREG",{0x7840001A,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx011010  fcun.w $w0, $w0, $w0
{          "fcune.d WREG , WREG , WREG",{0x78A0001C,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx011100  fcune.d $w0, $w0, $w0
{          "fcune.w WREG , WREG , WREG",{0x7880001C,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx011100  fcune.w $w0, $w0, $w0
{           "fdiv.d WREG , WREG , WREG",{0x78E0001B,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx011011  fdiv.d $w0, $w0, $w0
{           "fdiv.w WREG , WREG , WREG",{0x78C0001B,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx011011  fdiv.w $w0, $w0, $w0
{          "fexdo.h WREG , WREG , WREG",{0x7A00001B,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx011011  fexdo.h $w0, $w0, $w0
{          "fexdo.w WREG , WREG , WREG",{0x7A20001B,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx011011  fexdo.w $w0, $w0, $w0
{          "fexp2.d WREG , WREG , WREG",{0x79E0001B,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx011011  fexp2.d $w0, $w0, $w0
{          "fexp2.w WREG , WREG , WREG",{0x79C0001B,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx011011  fexp2.w $w0, $w0, $w0
{                "fexupl.d WREG , WREG",{0x7B31001E,0x0000FFC0}}, // 0111101100110001xxxxxxxxxx011110  fexupl.d $w0, $w0
{                "fexupl.w WREG , WREG",{0x7B30001E,0x0000FFC0}}, // 0111101100110000xxxxxxxxxx011110  fexupl.w $w0, $w0
{                "fexupr.d WREG , WREG",{0x7B33001E,0x0000FFC0}}, // 0111101100110011xxxxxxxxxx011110  fexupr.d $w0, $w0
{                "fexupr.w WREG , WREG",{0x7B32001E,0x0000FFC0}}, // 0111101100110010xxxxxxxxxx011110  fexupr.w $w0, $w0
{               "ffint_s.d WREG , WREG",{0x7B3D001E,0x0000FFC0}}, // 0111101100111101xxxxxxxxxx011110  ffint_s.d $w0, $w0
{               "ffint_s.w WREG , WREG",{0x7B3C001E,0x0000FFC0}}, // 0111101100111100xxxxxxxxxx011110  ffint_s.w $w0, $w0
{               "ffint_u.d WREG , WREG",{0x7B3F001E,0x0000FFC0}}, // 0111101100111111xxxxxxxxxx011110  ffint_u.d $w0, $w0
{               "ffint_u.w WREG , WREG",{0x7B3E001E,0x0000FFC0}}, // 0111101100111110xxxxxxxxxx011110  ffint_u.w $w0, $w0
{                  "ffql.d WREG , WREG",{0x7B35001E,0x0000FFC0}}, // 0111101100110101xxxxxxxxxx011110  ffql.d $w0, $w0
{                  "ffql.w WREG , WREG",{0x7B34001E,0x0000FFC0}}, // 0111101100110100xxxxxxxxxx011110  ffql.w $w0, $w0
{                  "ffqr.d WREG , WREG",{0x7B37001E,0x0000FFC0}}, // 0111101100110111xxxxxxxxxx011110  ffqr.d $w0, $w0
{                  "ffqr.w WREG , WREG",{0x7B36001E,0x0000FFC0}}, // 0111101100110110xxxxxxxxxx011110  ffqr.w $w0, $w0
{                 "fill.b WREG , GPREG",{0x7B00001E,0x0000FFC0}}, // 0111101100000000xxxxxxxxxx011110  fill.b $w0, $zero
{                 "fill.d WREG , GPREG",{0x7B03001E,0x0000FFC0}}, // 0111101100000011xxxxxxxxxx011110  fill.d $w0, $zero
{                 "fill.h WREG , GPREG",{0x7B01001E,0x0000FFC0}}, // 0111101100000001xxxxxxxxxx011110  fill.h $w0, $zero
{                 "fill.w WREG , GPREG",{0x7B02001E,0x0000FFC0}}, // 0111101100000010xxxxxxxxxx011110  fill.w $w0, $zero
{                 "flog2.d WREG , WREG",{0x7B2F001E,0x0000FFC0}}, // 0111101100101111xxxxxxxxxx011110  flog2.d $w0, $w0
{                 "flog2.w WREG , WREG",{0x7B2E001E,0x0000FFC0}}, // 0111101100101110xxxxxxxxxx011110  flog2.w $w0, $w0
{               "floor.w.d FREG , FREG",{0x4620000F,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx001111  floor.w.d $f0, $f0
{               "floor.w.s FREG , FREG",{0x4600000F,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx001111  floor.w.s $f0, $f0
{          "fmadd.d WREG , WREG , WREG",{0x7920001B,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx011011  fmadd.d $w0, $w0, $w0
{          "fmadd.w WREG , WREG , WREG",{0x7900001B,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx011011  fmadd.w $w0, $w0, $w0
{           "fmax.d WREG , WREG , WREG",{0x7BA0001B,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx011011  fmax.d $w0, $w0, $w0
{           "fmax.w WREG , WREG , WREG",{0x7B80001B,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx011011  fmax.w $w0, $w0, $w0
{         "fmax_a.d WREG , WREG , WREG",{0x7BE0001B,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx011011  fmax_a.d $w0, $w0, $w0
{         "fmax_a.w WREG , WREG , WREG",{0x7BC0001B,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx011011  fmax_a.w $w0, $w0, $w0
{           "fmin.d WREG , WREG , WREG",{0x7B20001B,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx011011  fmin.d $w0, $w0, $w0
{           "fmin.w WREG , WREG , WREG",{0x7B00001B,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx011011  fmin.w $w0, $w0, $w0
{         "fmin_a.d WREG , WREG , WREG",{0x7B60001B,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx011011  fmin_a.d $w0, $w0, $w0
{         "fmin_a.w WREG , WREG , WREG",{0x7B40001B,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx011011  fmin_a.w $w0, $w0, $w0
{          "fmsub.d WREG , WREG , WREG",{0x7960001B,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx011011  fmsub.d $w0, $w0, $w0
{          "fmsub.w WREG , WREG , WREG",{0x7940001B,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx011011  fmsub.w $w0, $w0, $w0
{           "fmul.d WREG , WREG , WREG",{0x78A0001B,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx011011  fmul.d $w0, $w0, $w0
{           "fmul.w WREG , WREG , WREG",{0x7880001B,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx011011  fmul.w $w0, $w0, $w0
{                  "frcp.d WREG , WREG",{0x7B2B001E,0x0000FFC0}}, // 0111101100101011xxxxxxxxxx011110  frcp.d $w0, $w0
{                  "frcp.w WREG , WREG",{0x7B2A001E,0x0000FFC0}}, // 0111101100101010xxxxxxxxxx011110  frcp.w $w0, $w0
{                 "frint.d WREG , WREG",{0x7B2D001E,0x0000FFC0}}, // 0111101100101101xxxxxxxxxx011110  frint.d $w0, $w0
{                 "frint.w WREG , WREG",{0x7B2C001E,0x0000FFC0}}, // 0111101100101100xxxxxxxxxx011110  frint.w $w0, $w0
{                "frsqrt.d WREG , WREG",{0x7B29001E,0x0000FFC0}}, // 0111101100101001xxxxxxxxxx011110  frsqrt.d $w0, $w0
{                "frsqrt.w WREG , WREG",{0x7B28001E,0x0000FFC0}}, // 0111101100101000xxxxxxxxxx011110  frsqrt.w $w0, $w0
{           "fsaf.d WREG , WREG , WREG",{0x7A20001A,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx011010  fsaf.d $w0, $w0, $w0
{           "fsaf.w WREG , WREG , WREG",{0x7A00001A,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx011010  fsaf.w $w0, $w0, $w0
{           "fseq.d WREG , WREG , WREG",{0x7AA0001A,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx011010  fseq.d $w0, $w0, $w0
{           "fseq.w WREG , WREG , WREG",{0x7A80001A,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx011010  fseq.w $w0, $w0, $w0
{           "fsle.d WREG , WREG , WREG",{0x7BA0001A,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx011010  fsle.d $w0, $w0, $w0
{           "fsle.w WREG , WREG , WREG",{0x7B80001A,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx011010  fsle.w $w0, $w0, $w0
{           "fslt.d WREG , WREG , WREG",{0x7B20001A,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx011010  fslt.d $w0, $w0, $w0
{           "fslt.w WREG , WREG , WREG",{0x7B00001A,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx011010  fslt.w $w0, $w0, $w0
{           "fsne.d WREG , WREG , WREG",{0x7AE0001C,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx011100  fsne.d $w0, $w0, $w0
{           "fsne.w WREG , WREG , WREG",{0x7AC0001C,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx011100  fsne.w $w0, $w0, $w0
{           "fsor.d WREG , WREG , WREG",{0x7A60001C,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx011100  fsor.d $w0, $w0, $w0
{           "fsor.w WREG , WREG , WREG",{0x7A40001C,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx011100  fsor.w $w0, $w0, $w0
{                 "fsqrt.d WREG , WREG",{0x7B27001E,0x0000FFC0}}, // 0111101100100111xxxxxxxxxx011110  fsqrt.d $w0, $w0
{                 "fsqrt.w WREG , WREG",{0x7B26001E,0x0000FFC0}}, // 0111101100100110xxxxxxxxxx011110  fsqrt.w $w0, $w0
{           "fsub.d WREG , WREG , WREG",{0x7860001B,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx011011  fsub.d $w0, $w0, $w0
{           "fsub.w WREG , WREG , WREG",{0x7840001B,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx011011  fsub.w $w0, $w0, $w0
{          "fsueq.d WREG , WREG , WREG",{0x7AE0001A,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx011010  fsueq.d $w0, $w0, $w0
{          "fsueq.w WREG , WREG , WREG",{0x7AC0001A,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx011010  fsueq.w $w0, $w0, $w0
{          "fsule.d WREG , WREG , WREG",{0x7BE0001A,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx011010  fsule.d $w0, $w0, $w0
{          "fsule.w WREG , WREG , WREG",{0x7BC0001A,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx011010  fsule.w $w0, $w0, $w0
{          "fsult.d WREG , WREG , WREG",{0x7B60001A,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx011010  fsult.d $w0, $w0, $w0
{          "fsult.w WREG , WREG , WREG",{0x7B40001A,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx011010  fsult.w $w0, $w0, $w0
{           "fsun.d WREG , WREG , WREG",{0x7A60001A,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx011010  fsun.d $w0, $w0, $w0
{           "fsun.w WREG , WREG , WREG",{0x7A40001A,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx011010  fsun.w $w0, $w0, $w0
{          "fsune.d WREG , WREG , WREG",{0x7AA0001C,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx011100  fsune.d $w0, $w0, $w0
{          "fsune.w WREG , WREG , WREG",{0x7A80001C,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx011100  fsune.w $w0, $w0, $w0
{               "ftint_s.d WREG , WREG",{0x7B39001E,0x0000FFC0}}, // 0111101100111001xxxxxxxxxx011110  ftint_s.d $w0, $w0
{               "ftint_s.w WREG , WREG",{0x7B38001E,0x0000FFC0}}, // 0111101100111000xxxxxxxxxx011110  ftint_s.w $w0, $w0
{               "ftint_u.d WREG , WREG",{0x7B3B001E,0x0000FFC0}}, // 0111101100111011xxxxxxxxxx011110  ftint_u.d $w0, $w0
{               "ftint_u.w WREG , WREG",{0x7B3A001E,0x0000FFC0}}, // 0111101100111010xxxxxxxxxx011110  ftint_u.w $w0, $w0
{            "ftq.h WREG , WREG , WREG",{0x7A80001B,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx011011  ftq.h $w0, $w0, $w0
{            "ftq.w WREG , WREG , WREG",{0x7AA0001B,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx011011  ftq.w $w0, $w0, $w0
{              "ftrunc_s.d WREG , WREG",{0x7B23001E,0x0000FFC0}}, // 0111101100100011xxxxxxxxxx011110  ftrunc_s.d $w0, $w0
{              "ftrunc_s.w WREG , WREG",{0x7B22001E,0x0000FFC0}}, // 0111101100100010xxxxxxxxxx011110  ftrunc_s.w $w0, $w0
{              "ftrunc_u.d WREG , WREG",{0x7B25001E,0x0000FFC0}}, // 0111101100100101xxxxxxxxxx011110  ftrunc_u.d $w0, $w0
{              "ftrunc_u.w WREG , WREG",{0x7B24001E,0x0000FFC0}}, // 0111101100100100xxxxxxxxxx011110  ftrunc_u.w $w0, $w0
{         "hadd_s.d WREG , WREG , WREG",{0x7A600015,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010101  hadd_s.d $w0, $w0, $w0
{         "hadd_s.h WREG , WREG , WREG",{0x7A200015,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010101  hadd_s.h $w0, $w0, $w0
{         "hadd_s.w WREG , WREG , WREG",{0x7A400015,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010101  hadd_s.w $w0, $w0, $w0
{         "hadd_u.d WREG , WREG , WREG",{0x7AE00015,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010101  hadd_u.d $w0, $w0, $w0
{         "hadd_u.h WREG , WREG , WREG",{0x7AA00015,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010101  hadd_u.h $w0, $w0, $w0
{         "hadd_u.w WREG , WREG , WREG",{0x7AC00015,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010101  hadd_u.w $w0, $w0, $w0
{         "hsub_s.d WREG , WREG , WREG",{0x7B600015,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx010101  hsub_s.d $w0, $w0, $w0
{         "hsub_s.h WREG , WREG , WREG",{0x7B200015,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx010101  hsub_s.h $w0, $w0, $w0
{         "hsub_s.w WREG , WREG , WREG",{0x7B400015,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx010101  hsub_s.w $w0, $w0, $w0
{         "hsub_u.d WREG , WREG , WREG",{0x7BE00015,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx010101  hsub_u.d $w0, $w0, $w0
{         "hsub_u.h WREG , WREG , WREG",{0x7BA00015,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx010101  hsub_u.h $w0, $w0, $w0
{         "hsub_u.w WREG , WREG , WREG",{0x7BC00015,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx010101  hsub_u.w $w0, $w0, $w0
{          "ilvev.b WREG , WREG , WREG",{0x7B000014,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx010100  ilvev.b $w0, $w0, $w0
{          "ilvev.d WREG , WREG , WREG",{0x7B600014,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx010100  ilvev.d $w0, $w0, $w0
{          "ilvev.h WREG , WREG , WREG",{0x7B200014,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx010100  ilvev.h $w0, $w0, $w0
{          "ilvev.w WREG , WREG , WREG",{0x7B400014,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx010100  ilvev.w $w0, $w0, $w0
{           "ilvl.b WREG , WREG , WREG",{0x7A000014,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx010100  ilvl.b $w0, $w0, $w0
{           "ilvl.d WREG , WREG , WREG",{0x7A600014,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx010100  ilvl.d $w0, $w0, $w0
{           "ilvl.h WREG , WREG , WREG",{0x7A200014,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx010100  ilvl.h $w0, $w0, $w0
{           "ilvl.w WREG , WREG , WREG",{0x7A400014,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx010100  ilvl.w $w0, $w0, $w0
{          "ilvod.b WREG , WREG , WREG",{0x7B800014,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx010100  ilvod.b $w0, $w0, $w0
{          "ilvod.d WREG , WREG , WREG",{0x7BE00014,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx010100  ilvod.d $w0, $w0, $w0
{          "ilvod.h WREG , WREG , WREG",{0x7BA00014,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx010100  ilvod.h $w0, $w0, $w0
{          "ilvod.w WREG , WREG , WREG",{0x7BC00014,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx010100  ilvod.w $w0, $w0, $w0
{           "ilvr.b WREG , WREG , WREG",{0x7A800014,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx010100  ilvr.b $w0, $w0, $w0
{           "ilvr.d WREG , WREG , WREG",{0x7AE00014,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx010100  ilvr.d $w0, $w0, $w0
{           "ilvr.h WREG , WREG , WREG",{0x7AA00014,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx010100  ilvr.h $w0, $w0, $w0
{           "ilvr.w WREG , WREG , WREG",{0x7AC00014,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx010100  ilvr.w $w0, $w0, $w0
{       "ins GPREG , GPREG , NUM , NUM",{0x7C000004,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx000100  ins $zero, $zero, 0, 1
{       "insert.b WREG [ NUM ] , GPREG",{0x79000019,0x000FFFC0}}, // 011110010000xxxxxxxxxxxxxx011001  insert.b $w0[0], $zero
{       "insert.d WREG [ NUM ] , GPREG",{0x79380019,0x0001FFC0}}, // 011110010011100xxxxxxxxxxx011001  insert.d $w0[0], $zero
{       "insert.h WREG [ NUM ] , GPREG",{0x79200019,0x0007FFC0}}, // 0111100100100xxxxxxxxxxxxx011001  insert.h $w0[0], $zero
{       "insert.w WREG [ NUM ] , GPREG",{0x79300019,0x0003FFC0}}, // 01111001001100xxxxxxxxxxxx011001  insert.w $w0[0], $zero
{                  "insv GPREG , GPREG",{0x7C00000C,0x03FF0000}}, // 011111xxxxxxxxxx0000000000001100  insv $zero, $zero
{ "insve.b WREG [ NUM ] , WREG [ NUM ]",{0x79400019,0x000FFFC0}}, // 011110010100xxxxxxxxxxxxxx011001  insve.b $w0[0], $w0[0]
{ "insve.d WREG [ NUM ] , WREG [ NUM ]",{0x79780019,0x0001FFC0}}, // 011110010111100xxxxxxxxxxx011001  insve.d $w0[0], $w0[0]
{ "insve.h WREG [ NUM ] , WREG [ NUM ]",{0x79600019,0x0007FFC0}}, // 0111100101100xxxxxxxxxxxxx011001  insve.h $w0[0], $w0[0]
{ "insve.w WREG [ NUM ] , WREG [ NUM ]",{0x79700019,0x0003FFC0}}, // 01111001011100xxxxxxxxxxxx011001  insve.w $w0[0], $w0[0]
{                               "j NUM",{0x08000000,0x03FFFFFF}}, // 000010xxxxxxxxxxxxxxxxxxxxxxxxxx  j 0
{                             "jal NUM",{0x0C000000,0x03FFFFFF}}, // 000011xxxxxxxxxxxxxxxxxxxxxxxxxx  jal 0
{                  "jalr GPREG , GPREG",{0x00000809,0x03E0F800}}, // 000000xxxxx00000xxxxx00000001001  jalr $at, $zero
{               "jalr.hb GPREG , GPREG",{0x00000C09,0x03E0F800}}, // 000000xxxxx00000xxxxx10000001001  jalr.hb $at, $zero
{                         "jialc NUM ,",{0xF8000000,0x001FFFFF}}, // 11111000000xxxxxxxxxxxxxxxxxxxxx  jialc 0,
{                           "jic NUM ,",{0xD8000000,0x001FFFFF}}, // 11011000000xxxxxxxxxxxxxxxxxxxxx  jic 0,
{                            "jr GPREG",{0x00000008,0x03E00001}}, // 000000xxxxx00000000000000000100x  jr $zero
{                         "jr.hb GPREG",{0x00000409,0x03E00000}}, // 000000xxxxx000000000010000001001  jr.hb $zero
{                "lb GPREG , ( GPREG )",{0x80000000,0x03FF0000}}, // 100000xxxxxxxxxx0000000000000000  lb $zero, ($zero)
{            "lb GPREG , NUM ( GPREG )",{0x80000100,0x03FFFFFF}}, // 100000xxxxxxxxxxxxxxxxxxxxxxxxxx  lb $zero, 0x100($zero)
{               "lbu GPREG , ( GPREG )",{0x90000000,0x03FF0000}}, // 100100xxxxxxxxxx0000000000000000  lbu $zero, ($zero)
{           "lbu GPREG , NUM ( GPREG )",{0x90000100,0x03FFFFFF}}, // 100100xxxxxxxxxxxxxxxxxxxxxxxxxx  lbu $zero, 0x100($zero)
{        "lbux GPREG , GPREG ( GPREG )",{0x7C00018A,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00110001010  lbux $zero, $zero($zero)
{               "ld.b WREG , ( GPREG )",{0x78000020,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100000  ld.b $w0, ($zero)
{           "ld.b WREG , NUM ( GPREG )",{0x7A000020,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100000  ld.b $w0, -0x200($zero)
{               "ld.d WREG , ( GPREG )",{0x78000023,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100011  ld.d $w0, ($zero)
{           "ld.d WREG , NUM ( GPREG )",{0x7A000023,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100011  ld.d $w0, -0x1000($zero)
{               "ld.h WREG , ( GPREG )",{0x78000021,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100001  ld.h $w0, ($zero)
{           "ld.h WREG , NUM ( GPREG )",{0x7A000021,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100001  ld.h $w0, -0x400($zero)
{               "ld.w WREG , ( GPREG )",{0x78000022,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100010  ld.w $w0, ($zero)
{           "ld.w WREG , NUM ( GPREG )",{0x7A000022,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100010  ld.w $w0, -0x800($zero)
{               "ldc1 FREG , ( GPREG )",{0xD4000000,0x03FF0000}}, // 110101xxxxxxxxxx0000000000000000  ldc1 $f0, ($zero)
{           "ldc1 FREG , NUM ( GPREG )",{0xD4000100,0x03FFFFFF}}, // 110101xxxxxxxxxxxxxxxxxxxxxxxxxx  ldc1 $f0, 0x100($zero)
{                     "ldc2 CASH , ( )",{0x49C00000,0x001F0000}}, // 01001001110xxxxx0000000000000000  ldc2 $0, ()
{                 "ldc2 CASH , ( NUM )",{0x49C00100,0x001FFFFF}}, // 01001001110xxxxxxxxxxxxxxxxxxxxx  ldc2 $0, (0x100)
{                    "ldi.b WREG , NUM",{0x7B000007,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx000111  ldi.b $w0, 0
{                    "ldi.d WREG , NUM",{0x7B600007,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx000111  ldi.d $w0, 0
{                    "ldi.h WREG , NUM",{0x7B200007,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx000111  ldi.h $w0, 0
{                    "ldi.w WREG , NUM",{0x7B400007,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx000111  ldi.w $w0, 0
{                "lh GPREG , ( GPREG )",{0x84000000,0x03FF0000}}, // 100001xxxxxxxxxx0000000000000000  lh $zero, ($zero)
{            "lh GPREG , NUM ( GPREG )",{0x84000100,0x03FFFFFF}}, // 100001xxxxxxxxxxxxxxxxxxxxxxxxxx  lh $zero, 0x100($zero)
{               "lhu GPREG , ( GPREG )",{0x94000000,0x03FF0000}}, // 100101xxxxxxxxxx0000000000000000  lhu $zero, ($zero)
{           "lhu GPREG , NUM ( GPREG )",{0x94000100,0x03FFFFFF}}, // 100101xxxxxxxxxxxxxxxxxxxxxxxxxx  lhu $zero, 0x100($zero)
{         "lhx GPREG , GPREG ( GPREG )",{0x7C00010A,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00100001010  lhx $zero, $zero($zero)
{                "ll GPREG , ( GPREG )",{0x7C000036,0x03FF0040}}, // 011111xxxxxxxxxx000000000x110110  ll $zero, ($zero)
{            "ll GPREG , NUM ( GPREG )",{0x7C000136,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx110110  ll $zero, 2($zero)
{               "lld GPREG , ( GPREG )",{0x7C000037,0x03FF0040}}, // 011111xxxxxxxxxx000000000x110111  lld $zero, ($zero)
{           "lld GPREG , NUM ( GPREG )",{0x7C000137,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx110111  lld $zero, 2($zero)
{     "lsa GPREG , GPREG , GPREG , NUM",{0x00000005,0x03FFF8C0}}, // 000000xxxxxxxxxxxxxxx000xx000101  lsa $zero, $zero, $zero, 0
{                "lw GPREG , ( GPREG )",{0x8C000000,0x03FF0000}}, // 100011xxxxxxxxxx0000000000000000  lw $zero, ($zero)
{            "lw GPREG , NUM ( GPREG )",{0x8C000100,0x03FFFFFF}}, // 100011xxxxxxxxxxxxxxxxxxxxxxxxxx  lw $zero, 0x100($zero)
{               "lwc1 FREG , ( GPREG )",{0xC4000000,0x03FF0000}}, // 110001xxxxxxxxxx0000000000000000  lwc1 $f0, ($zero)
{           "lwc1 FREG , NUM ( GPREG )",{0xC4000100,0x03FFFFFF}}, // 110001xxxxxxxxxxxxxxxxxxxxxxxxxx  lwc1 $f0, 0x100($zero)
{                     "lwc2 CASH , ( )",{0x49400000,0x001F0000}}, // 01001001010xxxxx0000000000000000  lwc2 $0, ()
{                 "lwc2 CASH , ( NUM )",{0x49400100,0x001FFFFF}}, // 01001001010xxxxxxxxxxxxxxxxxxxxx  lwc2 $0, (0x100)
{                    "lwpc GPREG , NUM",{0xEC080000,0x03E7FFFF}}, // 111011xxxxx01xxxxxxxxxxxxxxxxxxx  lwpc $zero, 0
{                   "lwupc GPREG , NUM",{0xEC100000,0x03E7FFFF}}, // 111011xxxxx10xxxxxxxxxxxxxxxxxxx  lwupc $zero, 0
{         "lwx GPREG , GPREG ( GPREG )",{0x7C00000A,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00000001010  lwx $zero, $zero($zero)
{          "madd ACREG , GPREG , GPREG",{0x70000000,0x03FF1800}}, // 011100xxxxxxxxxx000xx00000000000  madd $ac0, $zero, $zero
{         "madd_q.h WREG , WREG , WREG",{0x7940001C,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx011100  madd_q.h $w0, $w0, $w0
{         "madd_q.w WREG , WREG , WREG",{0x7960001C,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx011100  madd_q.w $w0, $w0, $w0
{          "maddf.d FREG , FREG , FREG",{0x46200018,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011000  maddf.d $f0, $f0, $f0
{          "maddf.s FREG , FREG , FREG",{0x46000018,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011000  maddf.s $f0, $f0, $f0
{        "maddr_q.h WREG , WREG , WREG",{0x7B40001C,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx011100  maddr_q.h $w0, $w0, $w0
{        "maddr_q.w WREG , WREG , WREG",{0x7B60001C,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx011100  maddr_q.w $w0, $w0, $w0
{         "maddu ACREG , GPREG , GPREG",{0x70000001,0x03FF1800}}, // 011100xxxxxxxxxx000xx00000000001  maddu $ac0, $zero, $zero
{          "maddv.b WREG , WREG , WREG",{0x78800012,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx010010  maddv.b $w0, $w0, $w0
{          "maddv.d WREG , WREG , WREG",{0x78E00012,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010010  maddv.d $w0, $w0, $w0
{          "maddv.h WREG , WREG , WREG",{0x78A00012,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010010  maddv.h $w0, $w0, $w0
{          "maddv.w WREG , WREG , WREG",{0x78C00012,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010010  maddv.w $w0, $w0, $w0
{   "maq_s.w.phl ACREG , GPREG , GPREG",{0x7C000530,0x03FF1800}}, // 011111xxxxxxxxxx000xx10100110000  maq_s.w.phl $ac0, $zero, $zero
{   "maq_s.w.phr ACREG , GPREG , GPREG",{0x7C0005B0,0x03FF1800}}, // 011111xxxxxxxxxx000xx10110110000  maq_s.w.phr $ac0, $zero, $zero
{  "maq_sa.w.phl ACREG , GPREG , GPREG",{0x7C000430,0x03FF1800}}, // 011111xxxxxxxxxx000xx10000110000  maq_sa.w.phl $ac0, $zero, $zero
{  "maq_sa.w.phr ACREG , GPREG , GPREG",{0x7C0004B0,0x03FF1800}}, // 011111xxxxxxxxxx000xx10010110000  maq_sa.w.phr $ac0, $zero, $zero
{            "max.d FREG , FREG , FREG",{0x4620001D,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011101  max.d $f0, $f0, $f0
{            "max.s FREG , FREG , FREG",{0x4600001D,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011101  max.s $f0, $f0, $f0
{          "max_a.b WREG , WREG , WREG",{0x7B00000E,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx001110  max_a.b $w0, $w0, $w0
{          "max_a.d WREG , WREG , WREG",{0x7B60000E,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx001110  max_a.d $w0, $w0, $w0
{          "max_a.h WREG , WREG , WREG",{0x7B20000E,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx001110  max_a.h $w0, $w0, $w0
{          "max_a.w WREG , WREG , WREG",{0x7B40000E,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx001110  max_a.w $w0, $w0, $w0
{          "max_s.b WREG , WREG , WREG",{0x7900000E,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx001110  max_s.b $w0, $w0, $w0
{          "max_s.d WREG , WREG , WREG",{0x7960000E,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx001110  max_s.d $w0, $w0, $w0
{          "max_s.h WREG , WREG , WREG",{0x7920000E,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx001110  max_s.h $w0, $w0, $w0
{          "max_s.w WREG , WREG , WREG",{0x7940000E,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx001110  max_s.w $w0, $w0, $w0
{          "max_u.b WREG , WREG , WREG",{0x7980000E,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx001110  max_u.b $w0, $w0, $w0
{          "max_u.d WREG , WREG , WREG",{0x79E0000E,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx001110  max_u.d $w0, $w0, $w0
{          "max_u.h WREG , WREG , WREG",{0x79A0000E,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx001110  max_u.h $w0, $w0, $w0
{          "max_u.w WREG , WREG , WREG",{0x79C0000E,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx001110  max_u.w $w0, $w0, $w0
{           "maxa.d FREG , FREG , FREG",{0x4620001F,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011111  maxa.d $f0, $f0, $f0
{           "maxa.s FREG , FREG , FREG",{0x4600001F,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011111  maxa.s $f0, $f0, $f0
{          "maxi_s.b WREG , WREG , NUM",{0x79000006,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx000110  maxi_s.b $w0, $w0, 0
{          "maxi_s.d WREG , WREG , NUM",{0x79600006,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx000110  maxi_s.d $w0, $w0, 0
{          "maxi_s.h WREG , WREG , NUM",{0x79200006,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx000110  maxi_s.h $w0, $w0, 0
{          "maxi_s.w WREG , WREG , NUM",{0x79400006,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx000110  maxi_s.w $w0, $w0, 0
{          "maxi_u.b WREG , WREG , NUM",{0x79800006,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx000110  maxi_u.b $w0, $w0, 0
{          "maxi_u.d WREG , WREG , NUM",{0x79E00006,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx000110  maxi_u.d $w0, $w0, 0
{          "maxi_u.h WREG , WREG , NUM",{0x79A00006,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx000110  maxi_u.h $w0, $w0, 0
{          "maxi_u.w WREG , WREG , NUM",{0x79C00006,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx000110  maxi_u.w $w0, $w0, 0
{            "mfc0 GPREG , GPREG , NUM",{0x40000000,0x001FF807}}, // 01000000000xxxxxxxxxx00000000xxx  mfc0 $zero, $zero, 0
{                   "mfc1 GPREG , FREG",{0x44000000,0x001FF800}}, // 01000100000xxxxxxxxxx00000000000  mfc1 $zero, $f0
{            "mfc2 GPREG , GPREG , NUM",{0x48000000,0x001FF807}}, // 01001000000xxxxxxxxxx00000000xxx  mfc2 $zero, $zero, 0
{                  "mfhc1 GPREG , FREG",{0x44600000,0x001FF000}}, // 01000100011xxxxxxxxx000000000000  mfhc1 $zero, $f0
{                  "mfhi GPREG , ACREG",{0x00000010,0x0060F800}}, // 000000000xx00000xxxxx00000010000  mfhi $zero, $ac0
{                  "mflo GPREG , ACREG",{0x00000012,0x0060F800}}, // 000000000xx00000xxxxx00000010010  mflo $zero, $ac0
{            "min.d FREG , FREG , FREG",{0x4620001C,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011100  min.d $f0, $f0, $f0
{            "min.s FREG , FREG , FREG",{0x4600001C,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011100  min.s $f0, $f0, $f0
{          "min_a.b WREG , WREG , WREG",{0x7B80000E,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx001110  min_a.b $w0, $w0, $w0
{          "min_a.d WREG , WREG , WREG",{0x7BE0000E,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx001110  min_a.d $w0, $w0, $w0
{          "min_a.h WREG , WREG , WREG",{0x7BA0000E,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx001110  min_a.h $w0, $w0, $w0
{          "min_a.w WREG , WREG , WREG",{0x7BC0000E,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx001110  min_a.w $w0, $w0, $w0
{          "min_s.b WREG , WREG , WREG",{0x7A00000E,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx001110  min_s.b $w0, $w0, $w0
{          "min_s.d WREG , WREG , WREG",{0x7A60000E,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx001110  min_s.d $w0, $w0, $w0
{          "min_s.h WREG , WREG , WREG",{0x7A20000E,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx001110  min_s.h $w0, $w0, $w0
{          "min_s.w WREG , WREG , WREG",{0x7A40000E,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx001110  min_s.w $w0, $w0, $w0
{          "min_u.b WREG , WREG , WREG",{0x7A80000E,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx001110  min_u.b $w0, $w0, $w0
{          "min_u.d WREG , WREG , WREG",{0x7AE0000E,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx001110  min_u.d $w0, $w0, $w0
{          "min_u.h WREG , WREG , WREG",{0x7AA0000E,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx001110  min_u.h $w0, $w0, $w0
{          "min_u.w WREG , WREG , WREG",{0x7AC0000E,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx001110  min_u.w $w0, $w0, $w0
{           "mina.d FREG , FREG , FREG",{0x4620001E,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011110  mina.d $f0, $f0, $f0
{           "mina.s FREG , FREG , FREG",{0x4600001E,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011110  mina.s $f0, $f0, $f0
{          "mini_s.b WREG , WREG , NUM",{0x7A000006,0x001FFFC0}}, // 01111010000xxxxxxxxxxxxxxx000110  mini_s.b $w0, $w0, 0
{          "mini_s.d WREG , WREG , NUM",{0x7A600006,0x001FFFC0}}, // 01111010011xxxxxxxxxxxxxxx000110  mini_s.d $w0, $w0, 0
{          "mini_s.h WREG , WREG , NUM",{0x7A200006,0x001FFFC0}}, // 01111010001xxxxxxxxxxxxxxx000110  mini_s.h $w0, $w0, 0
{          "mini_s.w WREG , WREG , NUM",{0x7A400006,0x001FFFC0}}, // 01111010010xxxxxxxxxxxxxxx000110  mini_s.w $w0, $w0, 0
{          "mini_u.b WREG , WREG , NUM",{0x7A800006,0x001FFFC0}}, // 01111010100xxxxxxxxxxxxxxx000110  mini_u.b $w0, $w0, 0
{          "mini_u.d WREG , WREG , NUM",{0x7AE00006,0x001FFFC0}}, // 01111010111xxxxxxxxxxxxxxx000110  mini_u.d $w0, $w0, 0
{          "mini_u.h WREG , WREG , NUM",{0x7AA00006,0x001FFFC0}}, // 01111010101xxxxxxxxxxxxxxx000110  mini_u.h $w0, $w0, 0
{          "mini_u.w WREG , WREG , NUM",{0x7AC00006,0x001FFFC0}}, // 01111010110xxxxxxxxxxxxxxx000110  mini_u.w $w0, $w0, 0
{           "mod GPREG , GPREG , GPREG",{0x000000DA,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00011011010  mod $zero, $zero, $zero
{          "mod_s.b WREG , WREG , WREG",{0x7B000012,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx010010  mod_s.b $w0, $w0, $w0
{          "mod_s.d WREG , WREG , WREG",{0x7B600012,0x001FFFC0}}, // 01111011011xxxxxxxxxxxxxxx010010  mod_s.d $w0, $w0, $w0
{          "mod_s.h WREG , WREG , WREG",{0x7B200012,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx010010  mod_s.h $w0, $w0, $w0
{          "mod_s.w WREG , WREG , WREG",{0x7B400012,0x001FFFC0}}, // 01111011010xxxxxxxxxxxxxxx010010  mod_s.w $w0, $w0, $w0
{          "mod_u.b WREG , WREG , WREG",{0x7B800012,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx010010  mod_u.b $w0, $w0, $w0
{          "mod_u.d WREG , WREG , WREG",{0x7BE00012,0x001FFFC0}}, // 01111011111xxxxxxxxxxxxxxx010010  mod_u.d $w0, $w0, $w0
{          "mod_u.h WREG , WREG , WREG",{0x7BA00012,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx010010  mod_u.h $w0, $w0, $w0
{          "mod_u.w WREG , WREG , WREG",{0x7BC00012,0x001FFFC0}}, // 01111011110xxxxxxxxxxxxxxx010010  mod_u.w $w0, $w0, $w0
{        "modsub GPREG , GPREG , GPREG",{0x7C000490,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10010010000  modsub $zero, $zero, $zero
{          "modu GPREG , GPREG , GPREG",{0x000000DB,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00011011011  modu $zero, $zero, $zero
{                   "mov.d FREG , FREG",{0x46200006,0x0000F780}}, // 0100011000100000xxxx0xxxx0000110  mov.d $f0, $f0
{                   "mov.s FREG , FREG",{0x46000006,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx000110  mov.s $f0, $f0
{                  "move GPREG , GPREG",{0x00000021,0x03E0F804}}, // 000000xxxxx00000xxxxx00000100x01  move $zero, $zero
{                  "move.v WREG , WREG",{0x78BE0019,0x0000FFC0}}, // 0111100010111110xxxxxxxxxx011001  move.v $w0, $w0
{          "msub ACREG , GPREG , GPREG",{0x70000004,0x03FF1800}}, // 011100xxxxxxxxxx000xx00000000100  msub $ac0, $zero, $zero
{         "msub_q.h WREG , WREG , WREG",{0x7980001C,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx011100  msub_q.h $w0, $w0, $w0
{         "msub_q.w WREG , WREG , WREG",{0x79A0001C,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx011100  msub_q.w $w0, $w0, $w0
{          "msubf.d FREG , FREG , FREG",{0x46200019,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx011001  msubf.d $f0, $f0, $f0
{          "msubf.s FREG , FREG , FREG",{0x46000019,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx011001  msubf.s $f0, $f0, $f0
{        "msubr_q.h WREG , WREG , WREG",{0x7B80001C,0x001FFFC0}}, // 01111011100xxxxxxxxxxxxxxx011100  msubr_q.h $w0, $w0, $w0
{        "msubr_q.w WREG , WREG , WREG",{0x7BA0001C,0x001FFFC0}}, // 01111011101xxxxxxxxxxxxxxx011100  msubr_q.w $w0, $w0, $w0
{         "msubu ACREG , GPREG , GPREG",{0x70000005,0x03FF1800}}, // 011100xxxxxxxxxx000xx00000000101  msubu $ac0, $zero, $zero
{          "msubv.b WREG , WREG , WREG",{0x79000012,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx010010  msubv.b $w0, $w0, $w0
{          "msubv.d WREG , WREG , WREG",{0x79600012,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010010  msubv.d $w0, $w0, $w0
{          "msubv.h WREG , WREG , WREG",{0x79200012,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010010  msubv.h $w0, $w0, $w0
{          "msubv.w WREG , WREG , WREG",{0x79400012,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010010  msubv.w $w0, $w0, $w0
{            "mtc0 GPREG , GPREG , NUM",{0x40800000,0x001FF807}}, // 01000000100xxxxxxxxxx00000000xxx  mtc0 $zero, $zero, 0
{                   "mtc1 GPREG , FREG",{0x44800000,0x001FF800}}, // 01000100100xxxxxxxxxx00000000000  mtc1 $zero, $f0
{            "mtc2 GPREG , GPREG , NUM",{0x48800000,0x001FF807}}, // 01001000100xxxxxxxxxx00000000xxx  mtc2 $zero, $zero, 0
{                  "mthc1 GPREG , FREG",{0x44E00000,0x001FF000}}, // 01000100111xxxxxxxxx000000000000  mthc1 $zero, $f0
{                  "mthi GPREG , ACREG",{0x00000011,0x03E01800}}, // 000000xxxxx00000000xx00000010001  mthi $zero, $ac0
{                "mthlip GPREG , ACREG",{0x7C0007F8,0x03E01800}}, // 011111xxxxx00000000xx11111111000  mthlip $zero, $ac0
{                  "mtlo GPREG , ACREG",{0x00000013,0x03E01800}}, // 000000xxxxx00000000xx00000010011  mtlo $zero, $ac0
{           "muh GPREG , GPREG , GPREG",{0x000000D8,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00011011000  muh $zero, $zero, $zero
{          "muhu GPREG , GPREG , GPREG",{0x000000D9,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00011011001  muhu $zero, $zero, $zero
{           "mul GPREG , GPREG , GPREG",{0x00000098,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00010011000  mul $zero, $zero, $zero
{            "mul.d FREG , FREG , FREG",{0x46200002,0x001EF780}}, // 01000110001xxxx0xxxx0xxxx0000010  mul.d $f0, $f0, $f0
{        "mul.ph GPREG , GPREG , GPREG",{0x7C000318,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01100011000  mul.ph $zero, $zero, $zero
{            "mul.s FREG , FREG , FREG",{0x46000002,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx000010  mul.s $f0, $f0, $f0
{          "mul_q.h WREG , WREG , WREG",{0x7900001C,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx011100  mul_q.h $w0, $w0, $w0
{          "mul_q.w WREG , WREG , WREG",{0x7920001C,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx011100  mul_q.w $w0, $w0, $w0
{      "mul_s.ph GPREG , GPREG , GPREG",{0x7C000398,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01110011000  mul_s.ph $zero, $zero, $zero
{ "muleq_s.w.phl GPREG , GPREG , GPREG",{0x7C000710,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11100010000  muleq_s.w.phl $zero, $zero, $zero
{ "muleq_s.w.phr GPREG , GPREG , GPREG",{0x7C000750,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11101010000  muleq_s.w.phr $zero, $zero, $zero
{"muleu_s.ph.qbl GPREG , GPREG , GPREG",{0x7C000190,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00110010000  muleu_s.ph.qbl $zero, $zero, $zero
{"muleu_s.ph.qbr GPREG , GPREG , GPREG",{0x7C0001D0,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00111010000  muleu_s.ph.qbr $zero, $zero, $zero
{    "mulq_rs.ph GPREG , GPREG , GPREG",{0x7C0007D0,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11111010000  mulq_rs.ph $zero, $zero, $zero
{     "mulq_rs.w GPREG , GPREG , GPREG",{0x7C0005D8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10111011000  mulq_rs.w $zero, $zero, $zero
{     "mulq_s.ph GPREG , GPREG , GPREG",{0x7C000790,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11110010000  mulq_s.ph $zero, $zero, $zero
{      "mulq_s.w GPREG , GPREG , GPREG",{0x7C000598,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10110011000  mulq_s.w $zero, $zero, $zero
{         "mulr_q.h WREG , WREG , WREG",{0x7B00001C,0x001FFFC0}}, // 01111011000xxxxxxxxxxxxxxx011100  mulr_q.h $w0, $w0, $w0
{         "mulr_q.w WREG , WREG , WREG",{0x7B20001C,0x001FFFC0}}, // 01111011001xxxxxxxxxxxxxxx011100  mulr_q.w $w0, $w0, $w0
{    "mulsa.w.ph ACREG , GPREG , GPREG",{0x7C0000B0,0x03FF1800}}, // 011111xxxxxxxxxx000xx00010110000  mulsa.w.ph $ac0, $zero, $zero
{ "mulsaq_s.w.ph ACREG , GPREG , GPREG",{0x7C0001B0,0x03FF1800}}, // 011111xxxxxxxxxx000xx00110110000  mulsaq_s.w.ph $ac0, $zero, $zero
{          "mult ACREG , GPREG , GPREG",{0x00000018,0x03FF1800}}, // 000000xxxxxxxxxx000xx00000011000  mult $ac0, $zero, $zero
{         "multu ACREG , GPREG , GPREG",{0x00000019,0x03FF1800}}, // 000000xxxxxxxxxx000xx00000011001  multu $ac0, $zero, $zero
{          "mulu GPREG , GPREG , GPREG",{0x00000099,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00010011001  mulu $zero, $zero, $zero
{           "mulv.b WREG , WREG , WREG",{0x78000012,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx010010  mulv.b $w0, $w0, $w0
{           "mulv.d WREG , WREG , WREG",{0x78600012,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010010  mulv.d $w0, $w0, $w0
{           "mulv.h WREG , WREG , WREG",{0x78200012,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010010  mulv.h $w0, $w0, $w0
{           "mulv.w WREG , WREG , WREG",{0x78400012,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010010  mulv.w $w0, $w0, $w0
{                   "neg GPREG , GPREG",{0x00000022,0x001FF800}}, // 00000000000xxxxxxxxxx00000100010  neg $zero, $zero
{                   "neg.d FREG , FREG",{0x46200007,0x0000F780}}, // 0100011000100000xxxx0xxxx0000111  neg.d $f0, $f0
{                   "neg.s FREG , FREG",{0x46000007,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx000111  neg.s $f0, $f0
{                  "negu GPREG , GPREG",{0x00000023,0x001FF800}}, // 00000000000xxxxxxxxxx00000100011  negu $zero, $zero
{                  "nloc.b WREG , WREG",{0x7B08001E,0x0000FFC0}}, // 0111101100001000xxxxxxxxxx011110  nloc.b $w0, $w0
{                  "nloc.d WREG , WREG",{0x7B0B001E,0x0000FFC0}}, // 0111101100001011xxxxxxxxxx011110  nloc.d $w0, $w0
{                  "nloc.h WREG , WREG",{0x7B09001E,0x0000FFC0}}, // 0111101100001001xxxxxxxxxx011110  nloc.h $w0, $w0
{                  "nloc.w WREG , WREG",{0x7B0A001E,0x0000FFC0}}, // 0111101100001010xxxxxxxxxx011110  nloc.w $w0, $w0
{                  "nlzc.b WREG , WREG",{0x7B0C001E,0x0000FFC0}}, // 0111101100001100xxxxxxxxxx011110  nlzc.b $w0, $w0
{                  "nlzc.d WREG , WREG",{0x7B0F001E,0x0000FFC0}}, // 0111101100001111xxxxxxxxxx011110  nlzc.d $w0, $w0
{                  "nlzc.h WREG , WREG",{0x7B0D001E,0x0000FFC0}}, // 0111101100001101xxxxxxxxxx011110  nlzc.h $w0, $w0
{                  "nlzc.w WREG , WREG",{0x7B0E001E,0x0000FFC0}}, // 0111101100001110xxxxxxxxxx011110  nlzc.w $w0, $w0
{                                 "nop",{0x00000000,0x00000000}}, // 00000000000000000000000000000000  nop
{           "nor GPREG , GPREG , GPREG",{0x00010027,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100111  nor $zero, $zero, $at
{            "nor.v WREG , WREG , WREG",{0x7840001E,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx011110  nor.v $w0, $w0, $w0
{            "nori.b WREG , WREG , NUM",{0x7A000000,0x00FFFFC0}}, // 01111010xxxxxxxxxxxxxxxxxx000000  nori.b $w0, $w0, 0
{                   "not GPREG , GPREG",{0x00000027,0x03E0F800}}, // 000000xxxxx00000xxxxx00000100111  not $zero, $zero
{            "or GPREG , GPREG , GPREG",{0x00010025,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100101  or $zero, $zero, $at
{             "or.v WREG , WREG , WREG",{0x7820001E,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx011110  or.v $w0, $w0, $w0
{             "ori GPREG , GPREG , NUM",{0x34000000,0x03FFFFFF}}, // 001101xxxxxxxxxxxxxxxxxxxxxxxxxx  ori $zero, $zero, 0
{             "ori.b WREG , WREG , NUM",{0x79000000,0x00FFFFC0}}, // 01111001xxxxxxxxxxxxxxxxxx000000  ori.b $w0, $w0, 0
{     "packrl.ph GPREG , GPREG , GPREG",{0x7C000391,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01110010001  packrl.ph $zero, $zero, $zero
{                               "pause",{0x00000140,0x00000000}}, // 00000000000000000000000101000000  pause
{          "pckev.b WREG , WREG , WREG",{0x79000014,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx010100  pckev.b $w0, $w0, $w0
{          "pckev.d WREG , WREG , WREG",{0x79600014,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010100  pckev.d $w0, $w0, $w0
{          "pckev.h WREG , WREG , WREG",{0x79200014,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010100  pckev.h $w0, $w0, $w0
{          "pckev.w WREG , WREG , WREG",{0x79400014,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010100  pckev.w $w0, $w0, $w0
{          "pckod.b WREG , WREG , WREG",{0x79800014,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx010100  pckod.b $w0, $w0, $w0
{          "pckod.d WREG , WREG , WREG",{0x79E00014,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx010100  pckod.d $w0, $w0, $w0
{          "pckod.h WREG , WREG , WREG",{0x79A00014,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx010100  pckod.h $w0, $w0, $w0
{          "pckod.w WREG , WREG , WREG",{0x79C00014,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx010100  pckod.w $w0, $w0, $w0
{                  "pcnt.b WREG , WREG",{0x7B04001E,0x0000FFC0}}, // 0111101100000100xxxxxxxxxx011110  pcnt.b $w0, $w0
{                  "pcnt.d WREG , WREG",{0x7B07001E,0x0000FFC0}}, // 0111101100000111xxxxxxxxxx011110  pcnt.d $w0, $w0
{                  "pcnt.h WREG , WREG",{0x7B05001E,0x0000FFC0}}, // 0111101100000101xxxxxxxxxx011110  pcnt.h $w0, $w0
{                  "pcnt.w WREG , WREG",{0x7B06001E,0x0000FFC0}}, // 0111101100000110xxxxxxxxxx011110  pcnt.w $w0, $w0
{       "pick.ph GPREG , GPREG , GPREG",{0x7C0002D1,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01011010001  pick.ph $zero, $zero, $zero
{       "pick.qb GPREG , GPREG , GPREG",{0x7C0000D1,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00011010001  pick.qb $zero, $zero, $zero
{          "preceq.w.phl GPREG , GPREG",{0x7C000312,0x001FF800}}, // 01111100000xxxxxxxxxx01100010010  preceq.w.phl $zero, $zero
{          "preceq.w.phr GPREG , GPREG",{0x7C000352,0x001FF800}}, // 01111100000xxxxxxxxxx01101010010  preceq.w.phr $zero, $zero
{        "precequ.ph.qbl GPREG , GPREG",{0x7C000112,0x001FF800}}, // 01111100000xxxxxxxxxx00100010010  precequ.ph.qbl $zero, $zero
{       "precequ.ph.qbla GPREG , GPREG",{0x7C000192,0x001FF800}}, // 01111100000xxxxxxxxxx00110010010  precequ.ph.qbla $zero, $zero
{        "precequ.ph.qbr GPREG , GPREG",{0x7C000152,0x001FF800}}, // 01111100000xxxxxxxxxx00101010010  precequ.ph.qbr $zero, $zero
{       "precequ.ph.qbra GPREG , GPREG",{0x7C0001D2,0x001FF800}}, // 01111100000xxxxxxxxxx00111010010  precequ.ph.qbra $zero, $zero
{         "preceu.ph.qbl GPREG , GPREG",{0x7C000712,0x001FF800}}, // 01111100000xxxxxxxxxx11100010010  preceu.ph.qbl $zero, $zero
{        "preceu.ph.qbla GPREG , GPREG",{0x7C000792,0x001FF800}}, // 01111100000xxxxxxxxxx11110010010  preceu.ph.qbla $zero, $zero
{         "preceu.ph.qbr GPREG , GPREG",{0x7C000752,0x001FF800}}, // 01111100000xxxxxxxxxx11101010010  preceu.ph.qbr $zero, $zero
{        "preceu.ph.qbra GPREG , GPREG",{0x7C0007D2,0x001FF800}}, // 01111100000xxxxxxxxxx11111010010  preceu.ph.qbra $zero, $zero
{   "precr.qb.ph GPREG , GPREG , GPREG",{0x7C000351,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01101010001  precr.qb.ph $zero, $zero, $zero
{  "precr_sra.ph.w GPREG , GPREG , NUM",{0x7C000791,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11110010001  precr_sra.ph.w $zero, $zero, 0
{"precr_sra_r.ph.w GPREG , GPREG , NUM",{0x7C0007D1,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11111010001  precr_sra_r.ph.w $zero, $zero, 0
{   "precrq.ph.w GPREG , GPREG , GPREG",{0x7C000511,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10100010001  precrq.ph.w $zero, $zero, $zero
{  "precrq.qb.ph GPREG , GPREG , GPREG",{0x7C000311,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01100010001  precrq.qb.ph $zero, $zero, $zero
{"precrq_rs.ph.w GPREG , GPREG , GPREG",{0x7C000551,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10101010001  precrq_rs.ph.w $zero, $zero, $zero
{"precrqu_s.qb.ph GPREG , GPREG , GPREG",{0x7C0003D1,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01111010001  precrqu_s.qb.ph $zero, $zero, $zero
{                          "pref , ( )",{0x7C000035,0x00000000}}, // 01111100000000000000000000110101  pref , ()
{                      "pref , ( NUM )",{0x7E000035,0x03E0FF80}}, // 011111xxxxx00000xxxxxxxxx0110101  pref , (0x100000)
{                      "pref , NUM ( )",{0x7C010035,0x001F0000}}, // 01111100000xxxxx0000000000110101  pref , 1()
{                  "pref , NUM ( NUM )",{0x7E010035,0x03FFFF80}}, // 011111xxxxxxxxxxxxxxxxxxx0110101  pref , 1(0x100000)
{         "prepend GPREG , GPREG , NUM",{0x7C000071,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00001110001  prepend $zero, $zero, 0
{            "raddu.w.qb GPREG , GPREG",{0x7C000510,0x03E0F800}}, // 011111xxxxx00000xxxxx10100010000  raddu.w.qb $zero, $zero
{                   "rddsp GPREG , NUM",{0x7C0004B8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10010111000  rddsp $zero, 0
{                  "rdhwr GPREG , CASH",{0x7C00E83B,0x001F0000}}, // 01111100000xxxxx1110100000111011  rdhwr $zero, $29
{                 "repl.ph GPREG , NUM",{0x7C000292,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01010010010  repl.ph $zero, 0
{                 "repl.qb GPREG , NUM",{0x7C000092,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00010010010  repl.qb $zero, 0
{              "replv.ph GPREG , GPREG",{0x7C0002D2,0x001FF800}}, // 01111100000xxxxxxxxxx01011010010  replv.ph $zero, $zero
{              "replv.qb GPREG , GPREG",{0x7C0000D2,0x001FF800}}, // 01111100000xxxxxxxxxx00011010010  replv.qb $zero, $zero
{                  "rint.d FREG , FREG",{0x4620001A,0x0000FFC0}}, // 0100011000100000xxxxxxxxxx011010  rint.d $f0, $f0
{                  "rint.s FREG , FREG",{0x4600001A,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx011010  rint.s $f0, $f0
{            "rotr GPREG , GPREG , NUM",{0x00200002,0x001FFFC0}}, // 00000000001xxxxxxxxxxxxxxx000010  rotr $zero, $zero, 0
{         "rotrv GPREG , GPREG , GPREG",{0x00000046,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00001000110  rotrv $zero, $zero, $zero
{               "round.w.d FREG , FREG",{0x4620000C,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx001100  round.w.d $f0, $f0
{               "round.w.s FREG , FREG",{0x4600000C,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx001100  round.w.s $f0, $f0
{           "sat_s.b WREG , WREG , NUM",{0x7870000A,0x0007FFC0}}, // 0111100001110xxxxxxxxxxxxx001010  sat_s.b $w0, $w0, 0
{           "sat_s.d WREG , WREG , NUM",{0x7800000A,0x003FFFC0}}, // 0111100000xxxxxxxxxxxxxxxx001010  sat_s.d $w0, $w0, 0
{           "sat_s.h WREG , WREG , NUM",{0x7860000A,0x000FFFC0}}, // 011110000110xxxxxxxxxxxxxx001010  sat_s.h $w0, $w0, 0
{           "sat_s.w WREG , WREG , NUM",{0x7840000A,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx001010  sat_s.w $w0, $w0, 0
{           "sat_u.b WREG , WREG , NUM",{0x78F0000A,0x0007FFC0}}, // 0111100011110xxxxxxxxxxxxx001010  sat_u.b $w0, $w0, 0
{           "sat_u.d WREG , WREG , NUM",{0x7880000A,0x003FFFC0}}, // 0111100010xxxxxxxxxxxxxxxx001010  sat_u.d $w0, $w0, 0
{           "sat_u.h WREG , WREG , NUM",{0x78E0000A,0x000FFFC0}}, // 011110001110xxxxxxxxxxxxxx001010  sat_u.h $w0, $w0, 0
{           "sat_u.w WREG , WREG , NUM",{0x78C0000A,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx001010  sat_u.w $w0, $w0, 0
{                "sb GPREG , ( GPREG )",{0xA0000000,0x03FF0000}}, // 101000xxxxxxxxxx0000000000000000  sb $zero, ($zero)
{            "sb GPREG , NUM ( GPREG )",{0xA0000100,0x03FFFFFF}}, // 101000xxxxxxxxxxxxxxxxxxxxxxxxxx  sb $zero, 0x100($zero)
{                "sc GPREG , ( GPREG )",{0x7C000026,0x03FF0040}}, // 011111xxxxxxxxxx000000000x100110  sc $zero, ($zero)
{            "sc GPREG , NUM ( GPREG )",{0x7C000126,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx100110  sc $zero, 2($zero)
{               "scd GPREG , ( GPREG )",{0x7C000027,0x03FF0040}}, // 011111xxxxxxxxxx000000000x100111  scd $zero, ($zero)
{           "scd GPREG , NUM ( GPREG )",{0x7C000127,0x03FFFFC0}}, // 011111xxxxxxxxxxxxxxxxxxxx100111  scd $zero, 2($zero)
{                               "sdbbp",{0x0000000E,0x00000000}}, // 00000000000000000000000000001110  sdbbp
{                           "sdbbp NUM",{0x0200000E,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx001110  sdbbp 0x80000
{               "sdc1 FREG , ( GPREG )",{0xF4000000,0x03FF0000}}, // 111101xxxxxxxxxx0000000000000000  sdc1 $f0, ($zero)
{           "sdc1 FREG , NUM ( GPREG )",{0xF4000100,0x03FFFFFF}}, // 111101xxxxxxxxxxxxxxxxxxxxxxxxxx  sdc1 $f0, 0x100($zero)
{                     "sdc2 CASH , ( )",{0x49E00000,0x001F0000}}, // 01001001111xxxxx0000000000000000  sdc2 $0, ()
{                 "sdc2 CASH , ( NUM )",{0x49E00100,0x001FFFFF}}, // 01001001111xxxxxxxxxxxxxxxxxxxxx  sdc2 $0, (0x100)
{                   "seb GPREG , GPREG",{0x7C000420,0x001FF800}}, // 01111100000xxxxxxxxxx10000100000  seb $zero, $zero
{                   "seh GPREG , GPREG",{0x7C000620,0x001FF800}}, // 01111100000xxxxxxxxxx11000100000  seh $zero, $zero
{            "sel.d FREG , FREG , FREG",{0x46200010,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx010000  sel.d $f0, $f0, $f0
{            "sel.s FREG , FREG , FREG",{0x46000010,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx010000  sel.s $f0, $f0, $f0
{         "seleqz.d FREG , FREG , FREG",{0x46200014,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx010100  seleqz.d $f0, $f0, $f0
{         "seleqz.s FREG , FREG , FREG",{0x46000014,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx010100  seleqz.s $f0, $f0, $f0
{         "selnez.d FREG , FREG , FREG",{0x46200017,0x001FFFC0}}, // 01000110001xxxxxxxxxxxxxxx010111  selnez.d $f0, $f0, $f0
{         "selnez.s FREG , FREG , FREG",{0x46000017,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx010111  selnez.s $f0, $f0, $f0
{                "sh GPREG , ( GPREG )",{0xA4000000,0x03FF0000}}, // 101001xxxxxxxxxx0000000000000000  sh $zero, ($zero)
{            "sh GPREG , NUM ( GPREG )",{0xA4000100,0x03FFFFFF}}, // 101001xxxxxxxxxxxxxxxxxxxxxxxxxx  sh $zero, 0x100($zero)
{             "shf.b WREG , WREG , NUM",{0x78000002,0x00FFFFC0}}, // 01111000xxxxxxxxxxxxxxxxxx000010  shf.b $w0, $w0, 0
{             "shf.h WREG , WREG , NUM",{0x79000002,0x00FFFFC0}}, // 01111001xxxxxxxxxxxxxxxxxx000010  shf.h $w0, $w0, 0
{             "shf.w WREG , WREG , NUM",{0x7A000002,0x00FFFFC0}}, // 01111010xxxxxxxxxxxxxxxxxx000010  shf.w $w0, $w0, 0
{                   "shilo ACREG , NUM",{0x7C0006B8,0x03F01800}}, // 011111xxxxxx0000000xx11010111000  shilo $ac0, 0
{                "shilov ACREG , GPREG",{0x7C0006F8,0x03E01800}}, // 011111xxxxx00000000xx11011111000  shilov $ac0, $zero
{         "shll.ph GPREG , GPREG , NUM",{0x7C000213,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01000010011  shll.ph $zero, $zero, 0
{         "shll.qb GPREG , GPREG , NUM",{0x7C000013,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00000010011  shll.qb $zero, $zero, 0
{       "shll_s.ph GPREG , GPREG , NUM",{0x7C000313,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01100010011  shll_s.ph $zero, $zero, 0
{        "shll_s.w GPREG , GPREG , NUM",{0x7C000513,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10100010011  shll_s.w $zero, $zero, 0
{      "shllv.ph GPREG , GPREG , GPREG",{0x7C000293,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01010010011  shllv.ph $zero, $zero, $zero
{      "shllv.qb GPREG , GPREG , GPREG",{0x7C000093,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00010010011  shllv.qb $zero, $zero, $zero
{    "shllv_s.ph GPREG , GPREG , GPREG",{0x7C000393,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01110010011  shllv_s.ph $zero, $zero, $zero
{     "shllv_s.w GPREG , GPREG , GPREG",{0x7C000593,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10110010011  shllv_s.w $zero, $zero, $zero
{         "shra.ph GPREG , GPREG , NUM",{0x7C000253,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01001010011  shra.ph $zero, $zero, 0
{         "shra.qb GPREG , GPREG , NUM",{0x7C000113,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00100010011  shra.qb $zero, $zero, 0
{       "shra_r.ph GPREG , GPREG , NUM",{0x7C000353,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01101010011  shra_r.ph $zero, $zero, 0
{       "shra_r.qb GPREG , GPREG , NUM",{0x7C000153,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00101010011  shra_r.qb $zero, $zero, 0
{        "shra_r.w GPREG , GPREG , NUM",{0x7C000553,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10101010011  shra_r.w $zero, $zero, 0
{      "shrav.ph GPREG , GPREG , GPREG",{0x7C0002D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01011010011  shrav.ph $zero, $zero, $zero
{      "shrav.qb GPREG , GPREG , GPREG",{0x7C000193,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00110010011  shrav.qb $zero, $zero, $zero
{    "shrav_r.ph GPREG , GPREG , GPREG",{0x7C0003D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01111010011  shrav_r.ph $zero, $zero, $zero
{    "shrav_r.qb GPREG , GPREG , GPREG",{0x7C0001D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00111010011  shrav_r.qb $zero, $zero, $zero
{     "shrav_r.w GPREG , GPREG , GPREG",{0x7C0005D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10111010011  shrav_r.w $zero, $zero, $zero
{         "shrl.ph GPREG , GPREG , NUM",{0x7C000653,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11001010011  shrl.ph $zero, $zero, 0
{         "shrl.qb GPREG , GPREG , NUM",{0x7C000053,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00001010011  shrl.qb $zero, $zero, 0
{      "shrlv.ph GPREG , GPREG , GPREG",{0x7C0006D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx11011010011  shrlv.ph $zero, $zero, $zero
{      "shrlv.qb GPREG , GPREG , GPREG",{0x7C0000D3,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00011010011  shrlv.qb $zero, $zero, $zero
{         "sld.b WREG , WREG [ GPREG ]",{0x78000014,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx010100  sld.b $w0, $w0[$zero]
{         "sld.d WREG , WREG [ GPREG ]",{0x78600014,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010100  sld.d $w0, $w0[$zero]
{         "sld.h WREG , WREG [ GPREG ]",{0x78200014,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010100  sld.h $w0, $w0[$zero]
{         "sld.w WREG , WREG [ GPREG ]",{0x78400014,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010100  sld.w $w0, $w0[$zero]
{          "sldi.b WREG , WREG [ NUM ]",{0x78000019,0x000FFFC0}}, // 011110000000xxxxxxxxxxxxxx011001  sldi.b $w0, $w0[0]
{          "sldi.d WREG , WREG [ NUM ]",{0x78380019,0x0001FFC0}}, // 011110000011100xxxxxxxxxxx011001  sldi.d $w0, $w0[0]
{          "sldi.h WREG , WREG [ NUM ]",{0x78200019,0x0007FFC0}}, // 0111100000100xxxxxxxxxxxxx011001  sldi.h $w0, $w0[0]
{          "sldi.w WREG , WREG [ NUM ]",{0x78300019,0x0003FFC0}}, // 01111000001100xxxxxxxxxxxx011001  sldi.w $w0, $w0[0]
{             "sll GPREG , GPREG , NUM",{0x00010000,0x001FFFC0}}, // 00000000000xxxxxxxxxxxxxxx000000  sll $zero, $at, 0
{            "sll.b WREG , WREG , WREG",{0x7800000D,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx001101  sll.b $w0, $w0, $w0
{            "sll.d WREG , WREG , WREG",{0x7860000D,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx001101  sll.d $w0, $w0, $w0
{            "sll.h WREG , WREG , WREG",{0x7820000D,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx001101  sll.h $w0, $w0, $w0
{            "sll.w WREG , WREG , WREG",{0x7840000D,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx001101  sll.w $w0, $w0, $w0
{            "slli.b WREG , WREG , NUM",{0x78700009,0x0007FFC0}}, // 0111100001110xxxxxxxxxxxxx001001  slli.b $w0, $w0, 0
{            "slli.d WREG , WREG , NUM",{0x78000009,0x003FFFC0}}, // 0111100000xxxxxxxxxxxxxxxx001001  slli.d $w0, $w0, 0
{            "slli.h WREG , WREG , NUM",{0x78600009,0x000FFFC0}}, // 011110000110xxxxxxxxxxxxxx001001  slli.h $w0, $w0, 0
{            "slli.w WREG , WREG , NUM",{0x78400009,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx001001  slli.w $w0, $w0, 0
{          "sllv GPREG , GPREG , GPREG",{0x00000004,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000000100  sllv $zero, $zero, $zero
{           "slt GPREG , GPREG , GPREG",{0x0000002A,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000101010  slt $zero, $zero, $zero
{            "slti GPREG , GPREG , NUM",{0x28000000,0x03FFFFFF}}, // 001010xxxxxxxxxxxxxxxxxxxxxxxxxx  slti $zero, $zero, 0
{           "sltiu GPREG , GPREG , NUM",{0x2C000000,0x03FFFFFF}}, // 001011xxxxxxxxxxxxxxxxxxxxxxxxxx  sltiu $zero, $zero, 0
{          "sltu GPREG , GPREG , GPREG",{0x0000002B,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000101011  sltu $zero, $zero, $zero
{       "splat.b WREG , WREG [ GPREG ]",{0x78800014,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx010100  splat.b $w0, $w0[$zero]
{       "splat.d WREG , WREG [ GPREG ]",{0x78E00014,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010100  splat.d $w0, $w0[$zero]
{       "splat.h WREG , WREG [ GPREG ]",{0x78A00014,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010100  splat.h $w0, $w0[$zero]
{       "splat.w WREG , WREG [ GPREG ]",{0x78C00014,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010100  splat.w $w0, $w0[$zero]
{        "splati.b WREG , WREG [ NUM ]",{0x78400019,0x000FFFC0}}, // 011110000100xxxxxxxxxxxxxx011001  splati.b $w0, $w0[0]
{        "splati.d WREG , WREG [ NUM ]",{0x78780019,0x0001FFC0}}, // 011110000111100xxxxxxxxxxx011001  splati.d $w0, $w0[0]
{        "splati.h WREG , WREG [ NUM ]",{0x78600019,0x0007FFC0}}, // 0111100001100xxxxxxxxxxxxx011001  splati.h $w0, $w0[0]
{        "splati.w WREG , WREG [ NUM ]",{0x78700019,0x0003FFC0}}, // 01111000011100xxxxxxxxxxxx011001  splati.w $w0, $w0[0]
{                  "sqrt.d FREG , FREG",{0x46200004,0x0000F780}}, // 0100011000100000xxxx0xxxx0000100  sqrt.d $f0, $f0
{                  "sqrt.s FREG , FREG",{0x46000004,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx000100  sqrt.s $f0, $f0
{             "sra GPREG , GPREG , NUM",{0x00000003,0x001FFFC0}}, // 00000000000xxxxxxxxxxxxxxx000011  sra $zero, $zero, 0
{            "sra.b WREG , WREG , WREG",{0x7880000D,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx001101  sra.b $w0, $w0, $w0
{            "sra.d WREG , WREG , WREG",{0x78E0000D,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx001101  sra.d $w0, $w0, $w0
{            "sra.h WREG , WREG , WREG",{0x78A0000D,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx001101  sra.h $w0, $w0, $w0
{            "sra.w WREG , WREG , WREG",{0x78C0000D,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx001101  sra.w $w0, $w0, $w0
{            "srai.b WREG , WREG , NUM",{0x78F00009,0x0007FFC0}}, // 0111100011110xxxxxxxxxxxxx001001  srai.b $w0, $w0, 0
{            "srai.d WREG , WREG , NUM",{0x78800009,0x003FFFC0}}, // 0111100010xxxxxxxxxxxxxxxx001001  srai.d $w0, $w0, 0
{            "srai.h WREG , WREG , NUM",{0x78E00009,0x000FFFC0}}, // 011110001110xxxxxxxxxxxxxx001001  srai.h $w0, $w0, 0
{            "srai.w WREG , WREG , NUM",{0x78C00009,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx001001  srai.w $w0, $w0, 0
{           "srar.b WREG , WREG , WREG",{0x78800015,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx010101  srar.b $w0, $w0, $w0
{           "srar.d WREG , WREG , WREG",{0x78E00015,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010101  srar.d $w0, $w0, $w0
{           "srar.h WREG , WREG , WREG",{0x78A00015,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010101  srar.h $w0, $w0, $w0
{           "srar.w WREG , WREG , WREG",{0x78C00015,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010101  srar.w $w0, $w0, $w0
{           "srari.b WREG , WREG , NUM",{0x7970000A,0x0007FFC0}}, // 0111100101110xxxxxxxxxxxxx001010  srari.b $w0, $w0, 0
{           "srari.d WREG , WREG , NUM",{0x7900000A,0x003FFFC0}}, // 0111100100xxxxxxxxxxxxxxxx001010  srari.d $w0, $w0, 0
{           "srari.h WREG , WREG , NUM",{0x7960000A,0x000FFFC0}}, // 011110010110xxxxxxxxxxxxxx001010  srari.h $w0, $w0, 0
{           "srari.w WREG , WREG , NUM",{0x7940000A,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx001010  srari.w $w0, $w0, 0
{          "srav GPREG , GPREG , GPREG",{0x00000007,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000000111  srav $zero, $zero, $zero
{             "srl GPREG , GPREG , NUM",{0x00000002,0x001FFFC0}}, // 00000000000xxxxxxxxxxxxxxx000010  srl $zero, $zero, 0
{            "srl.b WREG , WREG , WREG",{0x7900000D,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx001101  srl.b $w0, $w0, $w0
{            "srl.d WREG , WREG , WREG",{0x7960000D,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx001101  srl.d $w0, $w0, $w0
{            "srl.h WREG , WREG , WREG",{0x7920000D,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx001101  srl.h $w0, $w0, $w0
{            "srl.w WREG , WREG , WREG",{0x7940000D,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx001101  srl.w $w0, $w0, $w0
{            "srli.b WREG , WREG , NUM",{0x79700009,0x0007FFC0}}, // 0111100101110xxxxxxxxxxxxx001001  srli.b $w0, $w0, 0
{            "srli.d WREG , WREG , NUM",{0x79000009,0x003FFFC0}}, // 0111100100xxxxxxxxxxxxxxxx001001  srli.d $w0, $w0, 0
{            "srli.h WREG , WREG , NUM",{0x79600009,0x000FFFC0}}, // 011110010110xxxxxxxxxxxxxx001001  srli.h $w0, $w0, 0
{            "srli.w WREG , WREG , NUM",{0x79400009,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx001001  srli.w $w0, $w0, 0
{           "srlr.b WREG , WREG , WREG",{0x79000015,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx010101  srlr.b $w0, $w0, $w0
{           "srlr.d WREG , WREG , WREG",{0x79600015,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010101  srlr.d $w0, $w0, $w0
{           "srlr.h WREG , WREG , WREG",{0x79200015,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010101  srlr.h $w0, $w0, $w0
{           "srlr.w WREG , WREG , WREG",{0x79400015,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010101  srlr.w $w0, $w0, $w0
{           "srlri.b WREG , WREG , NUM",{0x79F0000A,0x0007FFC0}}, // 0111100111110xxxxxxxxxxxxx001010  srlri.b $w0, $w0, 0
{           "srlri.d WREG , WREG , NUM",{0x7980000A,0x003FFFC0}}, // 0111100110xxxxxxxxxxxxxxxx001010  srlri.d $w0, $w0, 0
{           "srlri.h WREG , WREG , NUM",{0x79E0000A,0x000FFFC0}}, // 011110011110xxxxxxxxxxxxxx001010  srlri.h $w0, $w0, 0
{           "srlri.w WREG , WREG , NUM",{0x79C0000A,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx001010  srlri.w $w0, $w0, 0
{          "srlv GPREG , GPREG , GPREG",{0x00000006,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000000110  srlv $zero, $zero, $zero
{                               "ssnop",{0x00000040,0x00000000}}, // 00000000000000000000000001000000  ssnop
{               "st.b WREG , ( GPREG )",{0x78000024,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100100  st.b $w0, ($zero)
{           "st.b WREG , NUM ( GPREG )",{0x7A000024,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100100  st.b $w0, -0x200($zero)
{               "st.d WREG , ( GPREG )",{0x78000027,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100111  st.d $w0, ($zero)
{           "st.d WREG , NUM ( GPREG )",{0x7A000027,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100111  st.d $w0, -0x1000($zero)
{               "st.h WREG , ( GPREG )",{0x78000025,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100101  st.h $w0, ($zero)
{           "st.h WREG , NUM ( GPREG )",{0x7A000025,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100101  st.h $w0, -0x400($zero)
{               "st.w WREG , ( GPREG )",{0x78000026,0x0000FFC0}}, // 0111100000000000xxxxxxxxxx100110  st.w $w0, ($zero)
{           "st.w WREG , NUM ( GPREG )",{0x7A000026,0x03FFFFC0}}, // 011110xxxxxxxxxxxxxxxxxxxx100110  st.w $w0, -0x800($zero)
{           "sub GPREG , GPREG , GPREG",{0x01000022,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100010  sub $zero, $t0, $zero
{            "sub.d FREG , FREG , FREG",{0x46200001,0x001EF780}}, // 01000110001xxxx0xxxx0xxxx0000001  sub.d $f0, $f0, $f0
{            "sub.s FREG , FREG , FREG",{0x46000001,0x001FFFC0}}, // 01000110000xxxxxxxxxxxxxxx000001  sub.s $f0, $f0, $f0
{       "subq.ph GPREG , GPREG , GPREG",{0x7C0002D0,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01011010000  subq.ph $zero, $zero, $zero
{     "subq_s.ph GPREG , GPREG , GPREG",{0x7C0003D0,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01111010000  subq_s.ph $zero, $zero, $zero
{      "subq_s.w GPREG , GPREG , GPREG",{0x7C0005D0,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10111010000  subq_s.w $zero, $zero, $zero
{      "subqh.ph GPREG , GPREG , GPREG",{0x7C000258,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01001011000  subqh.ph $zero, $zero, $zero
{       "subqh.w GPREG , GPREG , GPREG",{0x7C000458,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10001011000  subqh.w $zero, $zero, $zero
{    "subqh_r.ph GPREG , GPREG , GPREG",{0x7C0002D8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01011011000  subqh_r.ph $zero, $zero, $zero
{     "subqh_r.w GPREG , GPREG , GPREG",{0x7C0004D8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10011011000  subqh_r.w $zero, $zero, $zero
{         "subs_s.b WREG , WREG , WREG",{0x78000011,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx010001  subs_s.b $w0, $w0, $w0
{         "subs_s.d WREG , WREG , WREG",{0x78600011,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010001  subs_s.d $w0, $w0, $w0
{         "subs_s.h WREG , WREG , WREG",{0x78200011,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010001  subs_s.h $w0, $w0, $w0
{         "subs_s.w WREG , WREG , WREG",{0x78400011,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010001  subs_s.w $w0, $w0, $w0
{         "subs_u.b WREG , WREG , WREG",{0x78800011,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx010001  subs_u.b $w0, $w0, $w0
{         "subs_u.d WREG , WREG , WREG",{0x78E00011,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx010001  subs_u.d $w0, $w0, $w0
{         "subs_u.h WREG , WREG , WREG",{0x78A00011,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx010001  subs_u.h $w0, $w0, $w0
{         "subs_u.w WREG , WREG , WREG",{0x78C00011,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx010001  subs_u.w $w0, $w0, $w0
{       "subsus_u.b WREG , WREG , WREG",{0x79000011,0x001FFFC0}}, // 01111001000xxxxxxxxxxxxxxx010001  subsus_u.b $w0, $w0, $w0
{       "subsus_u.d WREG , WREG , WREG",{0x79600011,0x001FFFC0}}, // 01111001011xxxxxxxxxxxxxxx010001  subsus_u.d $w0, $w0, $w0
{       "subsus_u.h WREG , WREG , WREG",{0x79200011,0x001FFFC0}}, // 01111001001xxxxxxxxxxxxxxx010001  subsus_u.h $w0, $w0, $w0
{       "subsus_u.w WREG , WREG , WREG",{0x79400011,0x001FFFC0}}, // 01111001010xxxxxxxxxxxxxxx010001  subsus_u.w $w0, $w0, $w0
{       "subsuu_s.b WREG , WREG , WREG",{0x79800011,0x001FFFC0}}, // 01111001100xxxxxxxxxxxxxxx010001  subsuu_s.b $w0, $w0, $w0
{       "subsuu_s.d WREG , WREG , WREG",{0x79E00011,0x001FFFC0}}, // 01111001111xxxxxxxxxxxxxxx010001  subsuu_s.d $w0, $w0, $w0
{       "subsuu_s.h WREG , WREG , WREG",{0x79A00011,0x001FFFC0}}, // 01111001101xxxxxxxxxxxxxxx010001  subsuu_s.h $w0, $w0, $w0
{       "subsuu_s.w WREG , WREG , WREG",{0x79C00011,0x001FFFC0}}, // 01111001110xxxxxxxxxxxxxxx010001  subsuu_s.w $w0, $w0, $w0
{          "subu GPREG , GPREG , GPREG",{0x01000023,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100011  subu $zero, $t0, $zero
{       "subu.ph GPREG , GPREG , GPREG",{0x7C000250,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01001010000  subu.ph $zero, $zero, $zero
{       "subu.qb GPREG , GPREG , GPREG",{0x7C000050,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00001010000  subu.qb $zero, $zero, $zero
{     "subu_s.ph GPREG , GPREG , GPREG",{0x7C000350,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx01101010000  subu_s.ph $zero, $zero, $zero
{     "subu_s.qb GPREG , GPREG , GPREG",{0x7C000150,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00101010000  subu_s.qb $zero, $zero, $zero
{      "subuh.qb GPREG , GPREG , GPREG",{0x7C000058,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00001011000  subuh.qb $zero, $zero, $zero
{    "subuh_r.qb GPREG , GPREG , GPREG",{0x7C0000D8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx00011011000  subuh_r.qb $zero, $zero, $zero
{           "subv.b WREG , WREG , WREG",{0x7880000E,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx001110  subv.b $w0, $w0, $w0
{           "subv.d WREG , WREG , WREG",{0x78E0000E,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx001110  subv.d $w0, $w0, $w0
{           "subv.h WREG , WREG , WREG",{0x78A0000E,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx001110  subv.h $w0, $w0, $w0
{           "subv.w WREG , WREG , WREG",{0x78C0000E,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx001110  subv.w $w0, $w0, $w0
{           "subvi.b WREG , WREG , NUM",{0x78800006,0x001FFFC0}}, // 01111000100xxxxxxxxxxxxxxx000110  subvi.b $w0, $w0, 0
{           "subvi.d WREG , WREG , NUM",{0x78E00006,0x001FFFC0}}, // 01111000111xxxxxxxxxxxxxxx000110  subvi.d $w0, $w0, 0
{           "subvi.h WREG , WREG , NUM",{0x78A00006,0x001FFFC0}}, // 01111000101xxxxxxxxxxxxxxx000110  subvi.h $w0, $w0, 0
{           "subvi.w WREG , WREG , NUM",{0x78C00006,0x001FFFC0}}, // 01111000110xxxxxxxxxxxxxxx000110  subvi.w $w0, $w0, 0
{                "sw GPREG , ( GPREG )",{0xAC000000,0x03FF0000}}, // 101011xxxxxxxxxx0000000000000000  sw $zero, ($zero)
{            "sw GPREG , NUM ( GPREG )",{0xAC000100,0x03FFFFFF}}, // 101011xxxxxxxxxxxxxxxxxxxxxxxxxx  sw $zero, 0x100($zero)
{               "swc1 FREG , ( GPREG )",{0xE4000000,0x03FF0000}}, // 111001xxxxxxxxxx0000000000000000  swc1 $f0, ($zero)
{           "swc1 FREG , NUM ( GPREG )",{0xE4000100,0x03FFFFFF}}, // 111001xxxxxxxxxxxxxxxxxxxxxxxxxx  swc1 $f0, 0x100($zero)
{                     "swc2 CASH , ( )",{0x49600000,0x001F0000}}, // 01001001011xxxxx0000000000000000  swc2 $0, ()
{                 "swc2 CASH , ( NUM )",{0x49600100,0x001FFFFF}}, // 01001001011xxxxxxxxxxxxxxxxxxxxx  swc2 $0, (0x100)
{                                "sync",{0x0000000F,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000001111  sync
{                            "sync NUM",{0x0000010F,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx001111  sync 4
{                             "syscall",{0x0000000C,0x00000000}}, // 00000000000000000000000000001100  syscall
{                         "syscall NUM",{0x0200000C,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx001100  syscall 0x80000
{                   "teq GPREG , GPREG",{0x00000034,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110100  teq $zero, $zero
{             "teq GPREG , GPREG , NUM",{0x00000134,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110100  teq $zero, $zero, 4
{                   "tge GPREG , GPREG",{0x00000030,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110000  tge $zero, $zero
{             "tge GPREG , GPREG , NUM",{0x00000130,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110000  tge $zero, $zero, 4
{                  "tgeu GPREG , GPREG",{0x00000031,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110001  tgeu $zero, $zero
{            "tgeu GPREG , GPREG , NUM",{0x00000131,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110001  tgeu $zero, $zero, 4
{                                "tlbp",{0x42000008,0x00000000}}, // 01000010000000000000000000001000  tlbp
{                                "tlbr",{0x42000001,0x00000000}}, // 01000010000000000000000000000001  tlbr
{                               "tlbwi",{0x42000002,0x00000000}}, // 01000010000000000000000000000010  tlbwi
{                               "tlbwr",{0x42000006,0x00000000}}, // 01000010000000000000000000000110  tlbwr
{                   "tlt GPREG , GPREG",{0x00000032,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110010  tlt $zero, $zero
{             "tlt GPREG , GPREG , NUM",{0x00000132,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110010  tlt $zero, $zero, 4
{                  "tltu GPREG , GPREG",{0x00000033,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110011  tltu $zero, $zero
{            "tltu GPREG , GPREG , NUM",{0x00000133,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110011  tltu $zero, $zero, 4
{                   "tne GPREG , GPREG",{0x00000036,0x03FF0000}}, // 000000xxxxxxxxxx0000000000110110  tne $zero, $zero
{             "tne GPREG , GPREG , NUM",{0x00000136,0x03FFFFC0}}, // 000000xxxxxxxxxxxxxxxxxxxx110110  tne $zero, $zero, 4
{               "trunc.w.d FREG , FREG",{0x4620000D,0x0000F7C0}}, // 0100011000100000xxxx0xxxxx001101  trunc.w.d $f0, $f0
{               "trunc.w.s FREG , FREG",{0x4600000D,0x0000FFC0}}, // 0100011000000000xxxxxxxxxx001101  trunc.w.s $f0, $f0
{                               "undef",{0x01000000,0xFFFFFFFF}}, // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  undef
{           "vshf.b WREG , WREG , WREG",{0x78000015,0x001FFFC0}}, // 01111000000xxxxxxxxxxxxxxx010101  vshf.b $w0, $w0, $w0
{           "vshf.d WREG , WREG , WREG",{0x78600015,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx010101  vshf.d $w0, $w0, $w0
{           "vshf.h WREG , WREG , WREG",{0x78200015,0x001FFFC0}}, // 01111000001xxxxxxxxxxxxxxx010101  vshf.h $w0, $w0, $w0
{           "vshf.w WREG , WREG , WREG",{0x78400015,0x001FFFC0}}, // 01111000010xxxxxxxxxxxxxxx010101  vshf.w $w0, $w0, $w0
{                                "wait",{0x42000020,0x00000000}}, // 01000010000000000000000000100000  wait
{                   "wrdsp GPREG , NUM",{0x7C0004F8,0x03FFF800}}, // 011111xxxxxxxxxxxxxxx10011111000  wrdsp $zero, 0
{                  "wsbh GPREG , GPREG",{0x7C0000A0,0x001FF800}}, // 01111100000xxxxxxxxxx00010100000  wsbh $zero, $zero
{           "xor GPREG , GPREG , GPREG",{0x00000026,0x03FFF800}}, // 000000xxxxxxxxxxxxxxx00000100110  xor $zero, $zero, $zero
{            "xor.v WREG , WREG , WREG",{0x7860001E,0x001FFFC0}}, // 01111000011xxxxxxxxxxxxxxx011110  xor.v $w0, $w0, $w0
{            "xori GPREG , GPREG , NUM",{0x38000000,0x03FFFFFF}}, // 001110xxxxxxxxxxxxxxxxxxxxxxxxxx  xori $zero, $zero, 0
{            "xori.b WREG , WREG , NUM",{0x7B000000,0x00FFFFC0}}, // 01111011xxxxxxxxxxxxxxxxxx000000  xori.b $w0, $w0, 0
};

/*****************************************************************************/
/* capstone */
/*****************************************************************************/
const char *cs_err_to_string(cs_err e)
{
	switch(e) {
		case CS_ERR_OK: return "CS_ERR_OK";
		case CS_ERR_MEM: return "CS_ERR_MEM";
		case CS_ERR_ARCH: return "CS_ERR_ARCH";
		case CS_ERR_HANDLE: return "CS_ERR_HANDLE";
		case CS_ERR_CSH: return "CS_ERR_CSH";
		case CS_ERR_MODE: return "CS_ERR_MODE";
		case CS_ERR_OPTION: return "CS_ERR_OPTION";
		case CS_ERR_DETAIL: return "CS_ERR_DETAIL";
		case CS_ERR_MEMSETUP: return "CS_ERR_MEMSETUP";
		case CS_ERR_VERSION: return "CS_ERR_VERSION";
		case CS_ERR_DIET: return "CS_ERR_DIET";
		case CS_ERR_SKIPDATA: return "CS_ERR_SKIPDATA";
		case CS_ERR_X86_ATT: return "CS_ERR_X86_ATT";
		case CS_ERR_X86_INTEL: return "CS_ERR_X86_INTEL";
		default: return "WTF";
	}
}

int disasm(uint8_t *data, uint32_t addr, string& result, string& err)
{
	int rc = -1;
	static bool init = false;

	/* capstone vars */
	static csh handle;
	cs_insn *insn = NULL;
	size_t count;

	if (!init) {
		/* initialize capstone handle */
		cs_mode mode = (cs_mode)(CS_MODE_LITTLE_ENDIAN | CS_MODE_MIPS32R6);

		if(cs_open(CS_ARCH_MIPS, mode, &handle) != CS_ERR_OK) {
			printf("ERROR: cs_open()\n");
			goto cleanup;
		}
		init = true;
	}

	count = cs_disasm(handle, data,
		/* code_size */ 4,
		/* address */ addr,
		/* instr count */ 1,
		/* result */ &insn
	);

	if(count != 1) {
		cs_err e = cs_errno(handle);

		if(e == CS_ERR_OK) {
			result = "undefined";
		}
		else {
			char msg[128];
			sprintf(msg, "ERROR: cs_disasm() returned %zu cs_err=%d (%s)\n",
				count, e, cs_err_to_string(e));
			err = msg;
			goto cleanup;
		}
	}
	else {
		result = insn->mnemonic;
		if(insn->op_str[0]) {
			result += " ";
			result += insn->op_str;
		}
	}

	rc = 0;
	cleanup:
	if(insn) cs_free(insn, count);
	return rc;
}

/*****************************************************************************/
/* instruction tokenizing */
/*****************************************************************************/
enum tok_type {
	TT_GPREG,
	TT_FREG,
	TT_WREG,
	TT_ACREG,
	TT_CASH,
	TT_NUM,
	TT_PUNC,
	TT_OPCODE
};

struct token {
	tok_type type;
	uint32_t ival;
	string sval;
};

int tokenize(string src, vector<token>& result, string& err)
{
	int rc = -1, n=0;
	char *endptr;
	const char *inbuf = src.c_str();

	result.clear();

	/* grab opcode */
	while(inbuf[n]=='.' || inbuf[n]=='_' || isalnum(inbuf[n]))
		n++;
	result.push_back({TT_OPCODE, 0, string(inbuf, n)});
	inbuf += n;

	/* loop over the rest */
	int i=0;
	while(inbuf[0]) {
		char c = inbuf[0];
		char d = inbuf[1];
		char e = inbuf[2];

		/* skip spaces */
		if(c == ' ') {
			inbuf += 1;
		}
		/* tokens starting with dollar sign */
		else if(c=='$') {
			/* freg's */
			if(d=='f' && isdigit(e)) {
				uint32_t value = strtoul(inbuf+2, &endptr, 10);
				result.push_back({TT_FREG, value, ""});
				inbuf = endptr;
			}
			/* wreg's */
			else if(d=='w') {
				uint32_t value = strtoul(inbuf+2, &endptr, 10);
				result.push_back({TT_WREG, value, ""});
				inbuf = endptr;
			}
			/* acreg's */
			else if(d=='a' && e=='c') {
				uint32_t value = strtoul(inbuf+3, &endptr, 10);
				result.push_back({TT_ACREG, value, ""});
				inbuf = endptr;
			}
			/* $zero */
			else if(d=='z' && e=='e') {
				if(strncmp(inbuf, "$zero", 5)) {
					err = "expected $zero";
					MYLOG("ERROR: %s\n", err.c_str());
					goto cleanup;
				}
				result.push_back({TT_GPREG, 0, ""});
				inbuf += 5;
			}
			/* $0 and such */
			else if(isdigit(d)) {
				uint32_t value = strtoul(inbuf+1, &endptr, 10);
				result.push_back({TT_CASH, value, ""});
				inbuf = endptr;
			}
			/* we hope it's some other register we recognize */
			else {
				map<string,uint32_t> aliases = {
					{"$at",1}, {"$v0",2}, {"$v1",3},
					{"$a0",4}, {"$a1",5}, {"$a2",6}, {"$a3",7},
					{"$t0",8}, {"$t1",9}, {"$t2",10}, {"$t3",11},
					{"$t4",12}, {"$t5",13}, {"$t6",14}, {"$t7",15}, {"$t8",24}, {"$t9",25},
					{"$s0",16}, {"$s1",17}, {"$s2",18}, {"$s3",19},
					{"$s4",20}, {"$s5",21}, {"$s6",22}, {"$s7",23}, {"$s8",30},
					{"$k0",26}, {"$k1",27}, {"$gp",28}, {"$sp",29}, {"$fp",30}, {"$ra",31}
				};

				string reg = string(inbuf, 3);
				if(aliases.find(reg) == aliases.end()) {
					err = "unrecognized alias register: " + reg;
					MYLOG("ERROR: %s\n", err.c_str());
					goto cleanup;
				}

				result.push_back({TT_GPREG, aliases[reg], ""});

				inbuf += 3;
			}
		}
		/* hexadecimal immediates */
		else if((c=='0' && d=='x') || (c=='-' && d=='0' && e=='x')) {
			uint32_t value = strtoul(inbuf, &endptr, 16);
			result.push_back({TT_NUM, value, ""});
			inbuf = endptr;
		}
		/* decimal immediates */
		else if(isdigit(c) || (c=='-' && isdigit(d))) {
			uint32_t value = strtoul(inbuf, &endptr, 10);
			result.push_back({TT_NUM, value, ""});
			inbuf = endptr;
		}
		/* punctuation */
		else if(c=='[' || c==']' || c=='(' || c==')' || c==',' || c=='.' || c=='*' || c=='+' || c=='-') {
			result.push_back({TT_PUNC, 0, string(inbuf,1)});
			inbuf += 1;
		}
		/* wtf? */
		else {
			err = "error at: " + string(inbuf);
			MYLOG("ERROR: %s\n", err.c_str());
			goto cleanup;
		}
	}

	rc = 0;
	cleanup:
	return rc;
}

const char* token_type_tostr(int tt)
{
	switch(tt) {
		case TT_GPREG: return "GPREG";
		case TT_FREG: return "FREG";
		case TT_WREG: return "WREG";
		case TT_ACREG: return "ACREG";
		case TT_CASH: return "CASH";
		case TT_NUM: return "NUM";
		case TT_PUNC: return "PUNC";
		case TT_OPCODE: return "OPCODE";
		default:
			return "ERR_RESOLVING_TOKEN_TYPE";
	}
}

string tokens_to_syntax(vector<token>& tokens)
{
	string result;

	for(int i=0; i<tokens.size(); ++i) {
		if(i)
			result += " ";

		token t = tokens[i];

		switch(t.type) {
			case TT_GPREG:
			case TT_FREG:
			case TT_WREG:
			case TT_ACREG:
			case TT_CASH:
			case TT_NUM:
				result += token_type_tostr(t.type);
				break;
			case TT_PUNC:
			case TT_OPCODE:
				result += t.sval;
				break;
		}
	}

	return result;
}

/*****************************************************************************/
/* genetic */
/*****************************************************************************/

int count_bits(uint32_t x)
{
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	return ((((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24);
}

float hamming_similar(int a, int b)
{
	return (32-count_bits(a ^ b)) / 32.0f;
}

float fitness(vector<token> dst, vector<token> src) {
	int n = dst.size();

	/* same number of tokens */
	if(n != src.size())
		return 0;

	/* */
	float score = 0;
	float scorePerToken = 100.0 / n;

	/* for each token... */
	for(int i=0; i<n; ++i) {
		/* same type */
		if(src[i].type != dst[i].type)
			return 0;

		switch(src[i].type) {
			case TT_GPREG:
			case TT_FREG:
			case TT_WREG:
			case TT_ACREG:
			case TT_CASH:
			case TT_NUM:
				score += hamming_similar(src[i].ival, dst[i].ival) * scorePerToken;
				break;
			/* opcodes and flags must string match */
			case TT_OPCODE:
			case TT_PUNC:
				if(src[i].sval == dst[i].sval)
					score += scorePerToken;
				break;
			default:
				printf("ERROR!!!!\n");
		}
	}

	return score;
}

float score(vector<token> baseline, uint32_t newcomer, uint32_t addr)
{
	vector<token> toks_child;
	string err;
	string src;

	if(disasm((uint8_t *)&newcomer, addr, src, err))
		return -1;

	/* compare mnemonics before doing more work */
	string mnem = baseline[0].sval;
	if(src.compare(0, mnem.size(), mnem) != 0) {
		//printf("SHORTCUT!\n");
		return 0;
	}
	else {
		//printf("NOT SHORTCUT!\n");
	}

	/* mnemonics are the same, tokenize now... */
	if(tokenize(src, toks_child, err)) {
		printf("ERROR: %s\n", err.c_str());
		return 0;
	}

	return fitness(baseline, toks_child);
}

struct match {
	uint32_t src_hi, src_lo; // source bit range
	uint32_t dst_hi, dst_lo; // destination bit range
};

// force certain regions of bits to match
uint32_t enforce_bit_match(uint32_t inp, int bit, vector<match> matches)
{
	for(auto iter=matches.begin(); iter != matches.end(); ++iter) {
		struct match m = *iter;
		
		/* did we change a bit in the source region? */
		if(bit >= m.src_lo && bit <= m.src_hi) {
			/* compute masks */
			uint32_t a = 1<<m.src_hi;
			uint32_t b = 1<<m.src_lo;
			uint32_t src_mask = (a|(a-1)) ^ (b-1);
	
			a = 1<<m.dst_hi;
			b = 1<<m.dst_lo;
			uint32_t dst_mask = (a|(a-1)) ^ (b-1);
	
			/* mask and shift */
			inp = (inp & (~dst_mask));
			if(m.src_hi > m.dst_hi) {
				inp |= ((inp & src_mask) >> (m.src_hi - m.dst_hi));
			}
			else {
				inp |= ((inp & src_mask) << (m.dst_hi - m.src_hi));
			}
		}
	}

	return inp;
}

// for certain instructions with difficult inter-field dependencies
// inputs:
//     seed: the seed value that may need the special case
//  insword: instruction word
//      bit: last bit changed
uint32_t special_handling(uint32_t seed, uint32_t insword, int bit)
{
	switch(seed)
	{
		/* instructions need first two register fields to match */
		case 0x59080000: // bgezc
		case 0x19080000: // bgezalc
		case 0x1D080000: // bltzalc
		case 0x5D080000: // bltzc
		{
			vector<struct match> matches = {{25,21,20,16},{20,16,25,21}};
			return enforce_bit_match(insword, bit, matches);
		}
		default:
		return insword;
	}
}

#define N_OFFSPRING 1
#define N_BITS_FLIP 3
#define FAILURES_LIMIT 20000
int failures = 0;
int assemble_single(string src, uint32_t addr, uint8_t *result, string& err)
{
	int rc = -1;

	/* decompose instruction into tokens */
	vector<token> toks_src;
	vector<token> toks_child;

	if(tokenize(src, toks_src, err)) {
		err = "invalid syntax";
		return -1;
	}

	/* form syntax, look it up */
	string syn_src = tokens_to_syntax(toks_src);
	
	//printf("src:%s has syntax:%s\n", src.c_str(), syn_src.c_str());

	if(lookup.find(syn_src) == lookup.end()) {
		err = "invalid syntax (tried to look up: \"" + syn_src + "\")";
		return -1;
	}

	auto info = lookup[syn_src];
	uint32_t vary_mask = info.mask;

	/* for relative branches, shift the target address to 0 */
	if(toks_src[0].sval=="bnel" && toks_src.back().type == TT_NUM) {
		toks_src.back().ival -= (addr+4);
		addr = 0;
	}

	/* start with the parent */
	uint32_t parent = info.seed;
	float init_score, top_score;
	init_score = top_score = score(toks_src, parent, addr);

	/* cache the xor masks */
	int n_flips = 0;
	int flipper_idx[32];
	uint32_t flipper[32];
	for(int i=0; i<32; ++i) {
		if(vary_mask & (1 << i)) {
			flipper_idx[n_flips] = i;
			flipper[n_flips++] = 1<<i;
		}
	}

	failures = 0;
	int failstreak = 0;

	/* vary the parent */
	int b1i=0;
	while(1) {
		/* winner? */
		if(top_score > 99.99) {
			MYLOG("%08X wins!\n", parent);
			memcpy(result, &parent, 4);
			break;
		}

		bool overtake = false;

		for(; ; b1i = (b1i+1) % n_flips) {
			uint32_t child = parent ^ flipper[b1i];
			//printf("flipping bit: %08X, changing %08X -> %08X\n", flipper[b1i], parent, child);
			child = special_handling(info.seed, child, flipper_idx[b1i]);

			//MYLOG("b1i is now: %d\n", b1i);

			float s = score(toks_src, child, addr);
			if(s > top_score) {
				parent = child;
				top_score = s;
				overtake = true;
				b1i = (b1i+1 % n_flips);
				break;
			}

			if(0) {
				string tmp;
				disasm((uint8_t *)&child, addr, tmp, err);
				MYLOG("%08X: %s fails to overtake, score %f\n", child, tmp.c_str(), s);		
			}
	
			failures++;
			if(failures > FAILURES_LIMIT) {
				MYLOG("failure limit reached, not assembling!\n");
				err = "cannot assemble, valid operands?";
				goto cleanup;
			}

			failstreak++;
			//MYLOG("--failstreak is now: %d\n", failstreak);
			if(failstreak >= n_flips) {
				/* generate a new parent that's at least as good as the seed */
				while(1) {
					parent = info.seed;
					for(int i=0; i<n_flips; ++i) {
						if(rand()%2) {
							parent ^= flipper[i];
							parent = special_handling(info.seed, parent, flipper_idx[i]);
						}
					}

					top_score = score(toks_src, parent, addr);

					if(top_score >= init_score) {
						MYLOG("perturbing the parent to: %08X (score:%f) (vs:%f)\n", parent, top_score, init_score);
						break;
					}
					else {
						string tmp;
						disasm((uint8_t *)&parent, addr, tmp, err);
						MYLOG("%08X: %s perturb fail %f\n", parent, tmp.c_str(), top_score);		
						failures++;
					}

					if(failures > FAILURES_LIMIT) {
						err = "cannot assemble, valid operands?";
						MYLOG("failure limit reached, not assembling!\n");
						goto cleanup;
					}
				}
				failstreak = 0;
				break;
			}
		}

		if(overtake) {
			failstreak = 0;
			if(0) {
				string tmp;
				disasm((uint8_t *)&parent, addr, tmp, err);
				MYLOG("%08X: %s overtakes with 1-bit flip (%d) after %d failures, score %f\n", parent, tmp.c_str(), b1i, failures, top_score);		
			}
		}
	}

	rc = 0;
	cleanup:
	return rc;
}

/*****************************************************************************/
/* string processing crap */
/*****************************************************************************/

int split_newlines(const string& chunk, vector<string> &lines)
{
	lines.clear();

	const char *p = chunk.c_str();

	int left=0;
	for(int i=0; i<chunk.size(); ) {
		if(p[i]=='\x0a') {
			if(left < i) {
				lines.push_back(string(p,left,i-left));
			}

			left = i = (i+1);
		}
		else if(i+1 < chunk.size() && p[i]=='\x0d' && p[i+1]=='\x0a') {
			if(left < i) {
				lines.push_back(string(p,left,i-left));
			}

			left = i = (i+2);
		}
		else {
			i += 1;
		}
	}

	return 0;
}

int trim_lines(vector<string> &lines)
{
	vector<string> filtered;

	for(int i=0; i<lines.size(); ++i) {
		const char *p = lines[i].c_str();
		int left = 0, right = lines[i].size()-1;

		while(isspace(p[left]))
			left += 1;

		while(right>=0 && isspace(p[right]))
			right -= 1;

		if(right >= left)
			filtered.push_back(string(p, left, right-left+1));
	}

	lines = filtered;

	return 0;
}

// \S - one or more spaces
// \s - zero or more spaces
// \H - hex number (captures)
// \I - identifier (captures)
// \X - anything
bool fmt_match(string fmt, string str, vector<string>& result)
{
	bool match = false;
	int i=0, j=0;
	int nfmt=fmt.size(), nstr=str.size();

	result.clear();
	while(1) {
		string fcode = fmt.substr(i,2);

		if(fcode=="\\S") {
			if(!isspace(str[j]))
				goto cleanup;
			while(isspace(str[j]))
				j += 1;
			i += 2;
		}
		else
		if(fcode=="\\s") {
			while(isspace(str[j]))
				j += 1;
			i += 2;
		}
		else
		if(fcode=="\\I") {
			if(!isalpha(str[j]))
				goto cleanup;
			int start = j;
			j += 1;
			while(isalnum(str[j]))
				j += 1;
			result.push_back(str.substr(start, j-start));
			i += 2;
		}
		else
		if(fcode=="\\H") {
			const char *raw = str.c_str();
			char *endptr;
			strtoul(raw + j, &endptr, 16);
			int len = endptr - (raw+j);
			if(!len) goto cleanup;
			result.push_back(str.substr(j, len));
			i += 2;
			j += len;
		}
		else
		if(fcode=="\\X") {
			i += 2;
			j = nstr;
		}
		else
		if(fmt[i] == str[j]) {
			i += 1;
			j += 1;
		}
		else {
			goto cleanup;
		}

		if(i==nfmt && j==nstr) break;
	}

	match = true;
	cleanup:
	return match;
}

int assemble_multiline(const string& code, uint64_t addr, string& err)
{
	int rc = -1;
	vector<string> lines, fields;

	split_newlines(code, lines);
	trim_lines(lines);

	uint32_t vaddr;
	map<string,long> symbols;

	/* FIRST PASS */
	vaddr = addr;
	for(int i=0; i<lines.size(); ++i) {
		//printf("line %d: -%s-\n", i, lines[i].c_str());

		/* .org directive */
		if(fmt_match(".org\\S\\H", lines[i], fields)) {
			vaddr = strtol(fields[0].c_str(), 0, 16);
			if(vaddr & 0x3) {
				printf("ERROR: .org address is not 4-byte aligned\n");
				goto cleanup;
			}
			MYLOG("PASS1, set vaddr to: %08X\n", vaddr);
		}
		/* .equ directive */
		else
		if(fmt_match(".equ\\S\\I\\s,\\s\\H", lines[i], fields)) {
			uint32_t value = strtol(fields[1].c_str(), 0, 16);
			symbols[fields[0]] = value;
			printf("PASS1, set symbol %s: %08X\n", fields[0].c_str(), value);
		}
		/* labels */
		else
		if(fmt_match("\\I:", lines[i], fields)) {
			symbols[fields[0]] = vaddr;	
			printf("PASS1, set label %s: %08X\n", fields[0].c_str(), vaddr);
		}
	}

	/* SECOND PASS */
	vaddr = addr;
	for(int i=0; i<lines.size(); ++i) {
		vector<string> fields;

		//printf("line %d: -%s-\n", i, lines[i].c_str());

		/* .org directive */
		if(fmt_match(".org\\S\\H", lines[i], fields)) {
			vaddr = strtol(fields[0].c_str(), 0, 16);
			MYLOG("set vaddr to: %08X\n", vaddr);
		}
		/* .equ directive */
		else
		if(fmt_match(".equ\\S\\I\\s,\\s\\H", lines[i], fields)) {

		}
		/* labels */
		else
		if(fmt_match("\\I:", lines[i], fields)) {

		}
		/* comments */
		else
		if(fmt_match("\\s//\\X", lines[i], fields)) {
			
		}
		/* instructions */
		else {
			string err;
			uint8_t encoding[4];

			/* replace the last word (if it exists) with a label/symbol */
			string line = lines[i], token;
			int left = line.size()-1;
			while(left>=0 && isalnum(line[left]))
				left--;
			left += 1;
			token = line.substr(left, line.size()-left);
			if(fmt_match("\\I", token, fields)) {
				if(symbols.find(token) != symbols.end()) {
					char buf[16];
					long value = symbols[token];
					if(value < 0) {
						buf[0]='-';
						sprintf(buf+1, "0x%08X", (unsigned)(-1*value));
					}
					else {
						sprintf(buf, "0x%08X", (unsigned)value);
					}
					line.replace(left, line.size()-left, buf);
				}
				else {
					//printf("not found in symbol table\n");
				}
			}
			else {
				//printf("not an identifier\n");
			}

			/* now actually assemble */
			MYLOG("assembling: %s at address 0x%08X\n", line.c_str(), vaddr);
			if(assemble_single(line, vaddr, encoding, err)) {
				printf("ERROR on line: %s (%s)\n", lines[i].c_str(), err.c_str());
				break;
			}

			printf("%08X: %02X %02X %02X %02X\n", vaddr, encoding[0], encoding[1], encoding[2], encoding[3]);
			vaddr += 4;
		}
	}

	rc = 0;
	cleanup:
	return rc;
}

/*****************************************************************************/
/* main */
/*****************************************************************************/

#define TEST_ADDR 0x0

int main(int ac, char **av)
{
	int rc = -1;
	uint32_t insWord = 0x800000A;
	vector<token> tokens;
	uint8_t encoding[4];

	/* statistics crap */
	string srcWorstTime, srcWorstFails;
	clock_t t0, t1;
	double tdelta, tavg=0, tsum=0, tworst=0;
	int tcount = 0;
	uint32_t insWordWorstTime, insWordWorstFails;
	int failsWorst = 0;

	srand(time(NULL));

	/* decide mode */
	#define MODE_FILE 0
	#define MODE_RANDOM 1
	#define MODE_SINGLE 2
	int mode = MODE_RANDOM;
	if(ac > 1) {
		struct stat st;
		stat(av[1], &st);
		if(S_ISREG(st.st_mode)) {
			printf("FILE MODE!\n");
			mode = MODE_FILE;
		}
		else if(!strcmp(av[1], "random")) {
			printf("RANDOM MODE!\n");
			mode = MODE_RANDOM;
		}
		else {
			printf("SINGLE MODE!\n");
			mode = MODE_SINGLE;
		}
	}
	else {
		printf("need args!\n");
		goto cleanup;
	}

	if(mode == MODE_FILE) {
		char *line;
		string all, err;
		size_t len;

		FILE *fp = fopen(av[1], "r");
		if(!fp) {
			printf("ERROR: fopen(%s)\n", av[1]);
			goto cleanup;
		}

		while(getline(&line, &len, fp) != -1) {
			all += line;
		}

		fclose(fp);

		assemble_multiline(all, 0, err);

		return 0;	
	}

	if(mode == MODE_SINGLE) {
		string src, err;
		src = av[1];

		/* decompose instruction into tokens */
		vector<token> toks;

		if(tokenize(src, toks, err)) {
			printf("didn't even tokenize\n");
			return -1;
		}

		string syn = tokens_to_syntax(toks);
		printf(" input: %s\n", src.c_str());
		printf("syntax: %s\n", syn.c_str());

		t0 = clock();
		if(assemble_single(src, TEST_ADDR, encoding, err)) {
			printf("ERROR: %s\n", err.c_str());
			return -1;
		}
		tdelta = (double)(clock()-t0)/CLOCKS_PER_SEC;

		printf("assemble_single() duration: %fs (%f assembles/sec)\n", tdelta, 1/tdelta);
		printf("converged after %d failures to %08X\n", failures, *(uint32_t *)encoding);

		return 0;
	}

	if(mode == MODE_RANDOM) {
		string src, err;

		while(1) {
			insWord = (rand()<<16) | rand();
	
			if(0 != disasm((uint8_t *)&insWord, TEST_ADDR, src, err)) {
				printf("ERROR: %s\n", err.c_str());
				goto cleanup;
			}
	
			if(src == "undefined")
				continue;
	
			printf("%08X: %s\n", insWord, src.c_str());
	
			t0 = clock();
			if(assemble_single(src, TEST_ADDR, encoding, err)) {
				printf("ERROR: %s\n", err.c_str());
				printf("last instruction: '%s'\n", src.c_str());
				return -1;
			}
			tdelta = (double)(clock()-t0)/CLOCKS_PER_SEC;
			tsum += tdelta;
			tcount += 1;
			tavg = tsum/tcount;
			printf("assemble_single() duration: %fs, average: %fs (%f assembles/second)\n",
				tdelta, tavg, 1/tavg);
	
			if(tdelta > tworst) {
				insWordWorstTime = insWord;
				srcWorstTime = src;
				tworst = tdelta;
			}
	
			if(failures > failsWorst) {
				insWordWorstFails = insWord;
				failsWorst = failures;
				srcWorstFails = src;
			}
	
			printf("worst time: %f held by %08X: %s\n", tworst, insWordWorstTime, srcWorstTime.c_str());
			printf("worst fails: %d held by %08X: %s\n", failsWorst, insWordWorstFails, srcWorstFails.c_str());
		}

		return 0;
	}

	rc = 0;
	cleanup:
	return rc;
}
