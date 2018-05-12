
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
{                    "abs.d FREG , FREG",{0x46200005,0x0000F780}}, // 0110001000000100xxx0xxxx1010000x  abs.d $f0, $f0
{                    "abs.s FREG , FREG",{0x46000005,0x0000FFC0}}, // 0110001000000000xxxxxxxx101000xx  abs.s $f0, $f0
{              "absq_s.ph GPREG , GPREG",{0x7C000252,0x001FF800}}, // 00111110xxxxx000010xxxxx01001010  absq_s.ph $zero, $zero
{              "absq_s.qb GPREG , GPREG",{0x7C000052,0x001FF800}}, // 00111110xxxxx000000xxxxx01001010  absq_s.qb $zero, $zero
{               "absq_s.w GPREG , GPREG",{0x7C000452,0x001FF800}}, // 00111110xxxxx000001xxxxx01001010  absq_s.w $zero, $zero
{            "add GPREG , GPREG , GPREG",{0x00000020,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx00000100  add $zero, $zero, $zero
{             "add.d FREG , FREG , FREG",{0x46200000,0x001EF780}}, // 011000100xxxx100xxx0xxxx0000000x  add.d $f0, $f0, $f0
{             "add.s FREG , FREG , FREG",{0x46000000,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx000000xx  add.s $f0, $f0, $f0
{           "add_a.b WREG , WREG , WREG",{0x78000010,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx000010xx  add_a.b $w0, $w0, $w0
{           "add_a.d WREG , WREG , WREG",{0x78600010,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx000010xx  add_a.d $w0, $w0, $w0
{           "add_a.h WREG , WREG , WREG",{0x78200010,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx000010xx  add_a.h $w0, $w0, $w0
{           "add_a.w WREG , WREG , WREG",{0x78400010,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx000010xx  add_a.w $w0, $w0, $w0
{            "addiu GPREG , GPREG , NUM",{0x24000000,0x03FFFFFF}}, // xx100100xxxxxxxxxxxxxxxxxxxxxxxx  addiu $zero, $zero, 0
{                  "addiupc GPREG , NUM",{0xEC000000,0x03E7FFFF}}, // xx110111xxx00xxxxxxxxxxxxxxxxxxx  addiupc $zero, 0
{        "addq.ph GPREG , GPREG , GPREG",{0x7C000290,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00001001  addq.ph $zero, $zero, $zero
{      "addq_s.ph GPREG , GPREG , GPREG",{0x7C000390,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00001001  addq_s.ph $zero, $zero, $zero
{       "addq_s.w GPREG , GPREG , GPREG",{0x7C000590,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx00001001  addq_s.w $zero, $zero, $zero
{       "addqh.ph GPREG , GPREG , GPREG",{0x7C000218,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00011000  addqh.ph $zero, $zero, $zero
{        "addqh.w GPREG , GPREG , GPREG",{0x7C000418,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011000  addqh.w $zero, $zero, $zero
{     "addqh_r.ph GPREG , GPREG , GPREG",{0x7C000298,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00011001  addqh_r.ph $zero, $zero, $zero
{      "addqh_r.w GPREG , GPREG , GPREG",{0x7C000498,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011001  addqh_r.w $zero, $zero, $zero
{          "adds_a.b WREG , WREG , WREG",{0x78800010,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx000010xx  adds_a.b $w0, $w0, $w0
{          "adds_a.d WREG , WREG , WREG",{0x78E00010,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx000010xx  adds_a.d $w0, $w0, $w0
{          "adds_a.h WREG , WREG , WREG",{0x78A00010,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx000010xx  adds_a.h $w0, $w0, $w0
{          "adds_a.w WREG , WREG , WREG",{0x78C00010,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx000010xx  adds_a.w $w0, $w0, $w0
{          "adds_s.b WREG , WREG , WREG",{0x79000010,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx000010xx  adds_s.b $w0, $w0, $w0
{          "adds_s.d WREG , WREG , WREG",{0x79600010,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx000010xx  adds_s.d $w0, $w0, $w0
{          "adds_s.h WREG , WREG , WREG",{0x79200010,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx000010xx  adds_s.h $w0, $w0, $w0
{          "adds_s.w WREG , WREG , WREG",{0x79400010,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx000010xx  adds_s.w $w0, $w0, $w0
{          "adds_u.b WREG , WREG , WREG",{0x79800010,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx000010xx  adds_u.b $w0, $w0, $w0
{          "adds_u.d WREG , WREG , WREG",{0x79E00010,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx000010xx  adds_u.d $w0, $w0, $w0
{          "adds_u.h WREG , WREG , WREG",{0x79A00010,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx000010xx  adds_u.h $w0, $w0, $w0
{          "adds_u.w WREG , WREG , WREG",{0x79C00010,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx000010xx  adds_u.w $w0, $w0, $w0
{          "addsc GPREG , GPREG , GPREG",{0x7C000410,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00001000  addsc $zero, $zero, $zero
{           "addu GPREG , GPREG , GPREG",{0x00010021,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx10000100  addu $zero, $zero, $at
{        "addu.ph GPREG , GPREG , GPREG",{0x7C000210,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00001000  addu.ph $zero, $zero, $zero
{        "addu.qb GPREG , GPREG , GPREG",{0x7C000010,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00001000  addu.qb $zero, $zero, $zero
{      "addu_s.ph GPREG , GPREG , GPREG",{0x7C000310,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00001000  addu_s.ph $zero, $zero, $zero
{      "addu_s.qb GPREG , GPREG , GPREG",{0x7C000110,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx00001000  addu_s.qb $zero, $zero, $zero
{       "adduh.qb GPREG , GPREG , GPREG",{0x7C000018,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00011000  adduh.qb $zero, $zero, $zero
{     "adduh_r.qb GPREG , GPREG , GPREG",{0x7C000098,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00011001  adduh_r.qb $zero, $zero, $zero
{            "addv.b WREG , WREG , WREG",{0x7800000E,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx011100xx  addv.b $w0, $w0, $w0
{            "addv.d WREG , WREG , WREG",{0x7860000E,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx011100xx  addv.d $w0, $w0, $w0
{            "addv.h WREG , WREG , WREG",{0x7820000E,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx011100xx  addv.h $w0, $w0, $w0
{            "addv.w WREG , WREG , WREG",{0x7840000E,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx011100xx  addv.w $w0, $w0, $w0
{            "addvi.b WREG , WREG , NUM",{0x78000006,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx011000xx  addvi.b $w0, $w0, 0
{            "addvi.d WREG , WREG , NUM",{0x78600006,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx011000xx  addvi.d $w0, $w0, 0
{            "addvi.h WREG , WREG , NUM",{0x78200006,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx011000xx  addvi.h $w0, $w0, 0
{            "addvi.w WREG , WREG , NUM",{0x78400006,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx011000xx  addvi.w $w0, $w0, 0
{          "addwc GPREG , GPREG , GPREG",{0x7C000450,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00001010  addwc $zero, $zero, $zero
{    "align GPREG , GPREG , GPREG , NUM",{0x7C000220,0x03FFF8C0}}, // xx111110xxxxxxxx010xxxxx000001xx  align $zero, $zero, $zero, 0
{                   "aluipc GPREG , NUM",{0xEC1F0000,0x03E0FFFF}}, // xx11011111111xxxxxxxxxxxxxxxxxxx  aluipc $zero, 0
{            "and GPREG , GPREG , GPREG",{0x00000024,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx00100100  and $zero, $zero, $zero
{             "and.v WREG , WREG , WREG",{0x7800001E,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx011110xx  and.v $w0, $w0, $w0
{             "andi GPREG , GPREG , NUM",{0x30000000,0x03FFFFFF}}, // xx001100xxxxxxxxxxxxxxxxxxxxxxxx  andi $zero, $zero, 0
{             "andi.b WREG , WREG , NUM",{0x78000000,0x00FFFFC0}}, // 00011110xxxxxxxxxxxxxxxx000000xx  andi.b $w0, $w0, 0
{           "append GPREG , GPREG , NUM",{0x7C000031,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx10001100  append $zero, $zero, 0
{          "asub_s.b WREG , WREG , WREG",{0x7A000011,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx100010xx  asub_s.b $w0, $w0, $w0
{          "asub_s.d WREG , WREG , WREG",{0x7A600011,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx100010xx  asub_s.d $w0, $w0, $w0
{          "asub_s.h WREG , WREG , WREG",{0x7A200011,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx100010xx  asub_s.h $w0, $w0, $w0
{          "asub_s.w WREG , WREG , WREG",{0x7A400011,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx100010xx  asub_s.w $w0, $w0, $w0
{          "asub_u.b WREG , WREG , WREG",{0x7A800011,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx100010xx  asub_u.b $w0, $w0, $w0
{          "asub_u.d WREG , WREG , WREG",{0x7AE00011,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx100010xx  asub_u.d $w0, $w0, $w0
{          "asub_u.h WREG , WREG , WREG",{0x7AA00011,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx100010xx  asub_u.h $w0, $w0, $w0
{          "asub_u.w WREG , WREG , WREG",{0x7AC00011,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx100010xx  asub_u.w $w0, $w0, $w0
{              "aui GPREG , GPREG , NUM",{0x3C000000,0x03FFFFFF}}, // xx111100xxxxxxxxxxxxxxxxxxxxxxxx  aui $zero, $zero, 0
{                    "auipc GPREG , NUM",{0xEC1E0000,0x03E0FFFF}}, // xx11011101111xxxxxxxxxxxxxxxxxxx  auipc $zero, 0
{           "ave_s.b WREG , WREG , WREG",{0x7A000010,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx000010xx  ave_s.b $w0, $w0, $w0
{           "ave_s.d WREG , WREG , WREG",{0x7A600010,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx000010xx  ave_s.d $w0, $w0, $w0
{           "ave_s.h WREG , WREG , WREG",{0x7A200010,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx000010xx  ave_s.h $w0, $w0, $w0
{           "ave_s.w WREG , WREG , WREG",{0x7A400010,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx000010xx  ave_s.w $w0, $w0, $w0
{           "ave_u.b WREG , WREG , WREG",{0x7A800010,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx000010xx  ave_u.b $w0, $w0, $w0
{           "ave_u.d WREG , WREG , WREG",{0x7AE00010,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx000010xx  ave_u.d $w0, $w0, $w0
{           "ave_u.h WREG , WREG , WREG",{0x7AA00010,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx000010xx  ave_u.h $w0, $w0, $w0
{           "ave_u.w WREG , WREG , WREG",{0x7AC00010,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx000010xx  ave_u.w $w0, $w0, $w0
{          "aver_s.b WREG , WREG , WREG",{0x7B000010,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx000010xx  aver_s.b $w0, $w0, $w0
{          "aver_s.d WREG , WREG , WREG",{0x7B600010,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx000010xx  aver_s.d $w0, $w0, $w0
{          "aver_s.h WREG , WREG , WREG",{0x7B200010,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx000010xx  aver_s.h $w0, $w0, $w0
{          "aver_s.w WREG , WREG , WREG",{0x7B400010,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx000010xx  aver_s.w $w0, $w0, $w0
{          "aver_u.b WREG , WREG , WREG",{0x7B800010,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx000010xx  aver_u.b $w0, $w0, $w0
{          "aver_u.d WREG , WREG , WREG",{0x7BE00010,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx000010xx  aver_u.d $w0, $w0, $w0
{          "aver_u.h WREG , WREG , WREG",{0x7BA00010,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx000010xx  aver_u.h $w0, $w0, $w0
{          "aver_u.w WREG , WREG , WREG",{0x7BC00010,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx000010xx  aver_u.w $w0, $w0, $w0
{                                "b NUM",{0x10000000,0x0000FFFF}}, // 0000100000000000xxxxxxxxxxxxxxxx  b 4
{                              "bal NUM",{0x04110000,0x0000FFFF}}, // 0010000010001000xxxxxxxxxxxxxxxx  bal 4
{                             "balc NUM",{0xE8000000,0x03FFFFFF}}, // xx010111xxxxxxxxxxxxxxxxxxxxxxxx  balc 0
{           "balign GPREG , GPREG , NUM",{0x7C000431,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx10001100  balign $zero, $zero, 0
{                               "bc NUM",{0xC8000000,0x03FFFFFF}}, // xx010011xxxxxxxxxxxxxxxxxxxxxxxx  bc 0
{                    "bc1eqz FREG , NUM",{0x45200000,0x001FFFFF}}, // 10100010xxxxx100xxxxxxxxxxxxxxxx  bc1eqz $f0, 4
{                    "bc1nez FREG , NUM",{0x45A00000,0x001FFFFF}}, // 10100010xxxxx101xxxxxxxxxxxxxxxx  bc1nez $f0, 4
{                    "bc2eqz CASH , NUM",{0x49200000,0x001FFFFF}}, // 10010010xxxxx100xxxxxxxxxxxxxxxx  bc2eqz $0, 4
{                    "bc2nez CASH , NUM",{0x49A00000,0x001FFFFF}}, // 10010010xxxxx101xxxxxxxxxxxxxxxx  bc2nez $0, 4
{            "bclr.b WREG , WREG , WREG",{0x7980000D,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx101100xx  bclr.b $w0, $w0, $w0
{            "bclr.d WREG , WREG , WREG",{0x79E0000D,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx101100xx  bclr.d $w0, $w0, $w0
{            "bclr.h WREG , WREG , WREG",{0x79A0000D,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx101100xx  bclr.h $w0, $w0, $w0
{            "bclr.w WREG , WREG , WREG",{0x79C0000D,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx101100xx  bclr.w $w0, $w0, $w0
{            "bclri.b WREG , WREG , NUM",{0x79F00009,0x0007FFC0}}, // 10011110xxx01111xxxxxxxx100100xx  bclri.b $w0, $w0, 0
{            "bclri.d WREG , WREG , NUM",{0x79800009,0x003FFFC0}}, // 10011110xxxxxx01xxxxxxxx100100xx  bclri.d $w0, $w0, 0
{            "bclri.h WREG , WREG , NUM",{0x79E00009,0x000FFFC0}}, // 10011110xxxx0111xxxxxxxx100100xx  bclri.h $w0, $w0, 0
{            "bclri.w WREG , WREG , NUM",{0x79C00009,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx100100xx  bclri.w $w0, $w0, 0
{              "beq GPREG , GPREG , NUM",{0x10010000,0x03FFFFFF}}, // xx001000xxxxxxxxxxxxxxxxxxxxxxxx  beq $zero, $at, 4
{             "beqc GPREG , GPREG , NUM",{0x21090000,0x03FFFFFF}}, // xx000100xxxxxxxxxxxxxxxxxxxxxxxx  beqc $t0, $t1, 0
{             "beql GPREG , GPREG , NUM",{0x51000000,0x03FFFFFF}}, // xx001010xxxxxxxxxxxxxxxxxxxxxxxx  beql $t0, $zero, 4
{                     "beqz GPREG , NUM",{0x11000000,0x03E0FFFF}}, // xx00100000000xxxxxxxxxxxxxxxxxxx  beqz $t0, 4
{                  "beqzalc GPREG , NUM",{0x20010000,0x001FFFFF}}, // 00000100xxxxx000xxxxxxxxxxxxxxxx  beqzalc $at, 0
{                    "beqzc GPREG , NUM",{0xD9000000,0x03FFFFFF}}, // xx011011xxxxxxxxxxxxxxxxxxxxxxxx  beqzc $t0, 0
{                    "beqzl GPREG , NUM",{0x50000000,0x0000FFFF}}, // 0000101000000000xxxxxxxxxxxxxxxx  beqzl $zero, 4
{             "bgec GPREG , GPREG , NUM",{0x59010000,0x03FFFFFF}}, // xx011010xxxxxxxxxxxxxxxxxxxxxxxx  bgec $t0, $at, 0
{            "bgeuc GPREG , GPREG , NUM",{0x19010000,0x03FFFFFF}}, // xx011000xxxxxxxxxxxxxxxxxxxxxxxx  bgeuc $t0, $at, 0
{                     "bgez GPREG , NUM",{0x04010000,0x03E0FFFF}}, // xx10000010000xxxxxxxxxxxxxxxxxxx  bgez $zero, 4
{                  "bgezalc GPREG , NUM",{0x19080000,0x02F7FFFF}}, // 1x011000xxx1xxxxxxxxxxxxxxxxxxxx  bgezalc $t0, 0
{                  "bgezall GPREG , NUM",{0x04130000,0x03E0FFFF}}, // xx10000011001xxxxxxxxxxxxxxxxxxx  bgezall $zero, 4
{                    "bgezc GPREG , NUM",{0x59080000,0x02F7FFFF}}, // 1x011010xxx1xxxxxxxxxxxxxxxxxxxx  bgezc $t0, 0
{                    "bgezl GPREG , NUM",{0x04030000,0x03E0FFFF}}, // xx10000011000xxxxxxxxxxxxxxxxxxx  bgezl $zero, 4
{                     "bgtz GPREG , NUM",{0x1C000000,0x03E0FFFF}}, // xx11100000000xxxxxxxxxxxxxxxxxxx  bgtz $zero, 0
{                  "bgtzalc GPREG , NUM",{0x1C010000,0x001FFFFF}}, // 00111000xxxxx000xxxxxxxxxxxxxxxx  bgtzalc $at, 0
{                    "bgtzc GPREG , NUM",{0x5C010000,0x001FFFFF}}, // 00111010xxxxx000xxxxxxxxxxxxxxxx  bgtzc $at, 0
{                    "bgtzl GPREG , NUM",{0x5C000000,0x03E0FFFF}}, // xx11101000000xxxxxxxxxxxxxxxxxxx  bgtzl $zero, 4
{           "binsl.b WREG , WREG , WREG",{0x7B00000D,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx101100xx  binsl.b $w0, $w0, $w0
{           "binsl.d WREG , WREG , WREG",{0x7B60000D,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx101100xx  binsl.d $w0, $w0, $w0
{           "binsl.h WREG , WREG , WREG",{0x7B20000D,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx101100xx  binsl.h $w0, $w0, $w0
{           "binsl.w WREG , WREG , WREG",{0x7B40000D,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx101100xx  binsl.w $w0, $w0, $w0
{           "binsli.b WREG , WREG , NUM",{0x7B700009,0x0007FFC0}}, // 11011110xxx01110xxxxxxxx100100xx  binsli.b $w0, $w0, 0
{           "binsli.d WREG , WREG , NUM",{0x7B000009,0x003FFFC0}}, // 11011110xxxxxx00xxxxxxxx100100xx  binsli.d $w0, $w0, 0
{           "binsli.h WREG , WREG , NUM",{0x7B600009,0x000FFFC0}}, // 11011110xxxx0110xxxxxxxx100100xx  binsli.h $w0, $w0, 0
{           "binsli.w WREG , WREG , NUM",{0x7B400009,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx100100xx  binsli.w $w0, $w0, 0
{           "binsr.b WREG , WREG , WREG",{0x7B80000D,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx101100xx  binsr.b $w0, $w0, $w0
{           "binsr.d WREG , WREG , WREG",{0x7BE0000D,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx101100xx  binsr.d $w0, $w0, $w0
{           "binsr.h WREG , WREG , WREG",{0x7BA0000D,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx101100xx  binsr.h $w0, $w0, $w0
{           "binsr.w WREG , WREG , WREG",{0x7BC0000D,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx101100xx  binsr.w $w0, $w0, $w0
{           "binsri.b WREG , WREG , NUM",{0x7BF00009,0x0007FFC0}}, // 11011110xxx01111xxxxxxxx100100xx  binsri.b $w0, $w0, 0
{           "binsri.d WREG , WREG , NUM",{0x7B800009,0x003FFFC0}}, // 11011110xxxxxx01xxxxxxxx100100xx  binsri.d $w0, $w0, 0
{           "binsri.h WREG , WREG , NUM",{0x7BE00009,0x000FFFC0}}, // 11011110xxxx0111xxxxxxxx100100xx  binsri.h $w0, $w0, 0
{           "binsri.w WREG , WREG , NUM",{0x7BC00009,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx100100xx  binsri.w $w0, $w0, 0
{                 "bitrev GPREG , GPREG",{0x7C0006D2,0x001FF800}}, // 00111110xxxxx000011xxxxx01001011  bitrev $zero, $zero
{                "bitswap GPREG , GPREG",{0x7C000020,0x001FF800}}, // 00111110xxxxx000000xxxxx00000100  bitswap $zero, $zero
{                     "blez GPREG , NUM",{0x18000000,0x03E0FFFF}}, // xx01100000000xxxxxxxxxxxxxxxxxxx  blez $zero, 4
{                  "blezalc GPREG , NUM",{0x18010000,0x001FFFFF}}, // 00011000xxxxx000xxxxxxxxxxxxxxxx  blezalc $at, 0
{                    "blezc GPREG , NUM",{0x58010000,0x001FFFFF}}, // 00011010xxxxx000xxxxxxxxxxxxxxxx  blezc $at, 0
{                    "blezl GPREG , NUM",{0x58000000,0x03E0FFFF}}, // xx01101000000xxxxxxxxxxxxxxxxxxx  blezl $zero, 4
{             "bltc GPREG , GPREG , NUM",{0x5D010000,0x03FFFFFF}}, // xx111010xxxxxxxxxxxxxxxxxxxxxxxx  bltc $t0, $at, 0
{            "bltuc GPREG , GPREG , NUM",{0x1D010000,0x03FFFFFF}}, // xx111000xxxxxxxxxxxxxxxxxxxxxxxx  bltuc $t0, $at, 0
{                     "bltz GPREG , NUM",{0x04000000,0x03E0FFFF}}, // xx10000000000xxxxxxxxxxxxxxxxxxx  bltz $zero, 4
{                  "bltzalc GPREG , NUM",{0x1D080000,0x02F7FFFF}}, // 1x111000xxx1xxxxxxxxxxxxxxxxxxxx  bltzalc $t0, 0
{                  "bltzall GPREG , NUM",{0x04120000,0x03E0FFFF}}, // xx10000001001xxxxxxxxxxxxxxxxxxx  bltzall $zero, 4
{                    "bltzc GPREG , NUM",{0x5D080000,0x02F7FFFF}}, // 1x111010xxx1xxxxxxxxxxxxxxxxxxxx  bltzc $t0, 0
{                    "bltzl GPREG , NUM",{0x04020000,0x03E0FFFF}}, // xx10000001000xxxxxxxxxxxxxxxxxxx  bltzl $zero, 4
{            "bmnz.v WREG , WREG , WREG",{0x7880001E,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx011110xx  bmnz.v $w0, $w0, $w0
{            "bmnzi.b WREG , WREG , NUM",{0x78000001,0x00FFFFC0}}, // 00011110xxxxxxxxxxxxxxxx100000xx  bmnzi.b $w0, $w0, 0
{             "bmz.v WREG , WREG , WREG",{0x78A0001E,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx011110xx  bmz.v $w0, $w0, $w0
{             "bmzi.b WREG , WREG , NUM",{0x79000001,0x00FFFFC0}}, // 10011110xxxxxxxxxxxxxxxx100000xx  bmzi.b $w0, $w0, 0
{              "bne GPREG , GPREG , NUM",{0x14010000,0x03FFFFFF}}, // xx101000xxxxxxxxxxxxxxxxxxxxxxxx  bne $zero, $at, 4
{             "bnec GPREG , GPREG , NUM",{0x61090000,0x03FFFFFF}}, // xx000110xxxxxxxxxxxxxxxxxxxxxxxx  bnec $t0, $t1, 0
{            "bneg.b WREG , WREG , WREG",{0x7A80000D,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx101100xx  bneg.b $w0, $w0, $w0
{            "bneg.d WREG , WREG , WREG",{0x7AE0000D,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx101100xx  bneg.d $w0, $w0, $w0
{            "bneg.h WREG , WREG , WREG",{0x7AA0000D,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx101100xx  bneg.h $w0, $w0, $w0
{            "bneg.w WREG , WREG , WREG",{0x7AC0000D,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx101100xx  bneg.w $w0, $w0, $w0
{            "bnegi.b WREG , WREG , NUM",{0x7AF00009,0x0007FFC0}}, // 01011110xxx01111xxxxxxxx100100xx  bnegi.b $w0, $w0, 0
{            "bnegi.d WREG , WREG , NUM",{0x7A800009,0x003FFFC0}}, // 01011110xxxxxx01xxxxxxxx100100xx  bnegi.d $w0, $w0, 0
{            "bnegi.h WREG , WREG , NUM",{0x7AE00009,0x000FFFC0}}, // 01011110xxxx0111xxxxxxxx100100xx  bnegi.h $w0, $w0, 0
{            "bnegi.w WREG , WREG , NUM",{0x7AC00009,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx100100xx  bnegi.w $w0, $w0, 0
{             "bnel GPREG , GPREG , NUM",{0x54010000,0x03FFFFFF}}, // xx101010xxxxxxxxxxxxxxxxxxxxxxxx  bnel $zero, $at, 4
{                     "bnez GPREG , NUM",{0x14000000,0x03E0FFFF}}, // xx10100000000xxxxxxxxxxxxxxxxxxx  bnez $zero, 4
{                  "bnezalc GPREG , NUM",{0x60010000,0x001FFFFF}}, // 00000110xxxxx000xxxxxxxxxxxxxxxx  bnezalc $at, 0
{                    "bnezc GPREG , NUM",{0xF9000000,0x03FFFFFF}}, // xx011111xxxxxxxxxxxxxxxxxxxxxxxx  bnezc $t0, 0
{                    "bnezl GPREG , NUM",{0x54000000,0x03E0FFFF}}, // xx10101000000xxxxxxxxxxxxxxxxxxx  bnezl $zero, 4
{             "bnvc GPREG , GPREG , NUM",{0x60000000,0x03FFFFFF}}, // xx000110xxxxxxxxxxxxxxxxxxxxxxxx  bnvc $zero, $zero, 0
{                     "bnz.b WREG , NUM",{0x47800000,0x001FFFFF}}, // 11100010xxxxx001xxxxxxxxxxxxxxxx  bnz.b $w0, 4
{                     "bnz.d WREG , NUM",{0x47E00000,0x001FFFFF}}, // 11100010xxxxx111xxxxxxxxxxxxxxxx  bnz.d $w0, 4
{                     "bnz.h WREG , NUM",{0x47A00000,0x001FFFFF}}, // 11100010xxxxx101xxxxxxxxxxxxxxxx  bnz.h $w0, 4
{                     "bnz.v WREG , NUM",{0x45E00000,0x001FFFFF}}, // 10100010xxxxx111xxxxxxxxxxxxxxxx  bnz.v $w0, 4
{                     "bnz.w WREG , NUM",{0x47C00000,0x001FFFFF}}, // 11100010xxxxx011xxxxxxxxxxxxxxxx  bnz.w $w0, 4
{             "bovc GPREG , GPREG , NUM",{0x20000000,0x03FFFFFF}}, // xx000100xxxxxxxxxxxxxxxxxxxxxxxx  bovc $zero, $zero, 0
{                         "bposge32 NUM",{0x041C0000,0x0000FFFF}}, // 0010000000111000xxxxxxxxxxxxxxxx  bposge32 4
{                                "break",{0x0000000D,0x00000000}}, // 00000000000000000000000010110000  break
{                            "break NUM",{0x0200000D,0x03FF0000}}, // xx000000xxxxxxxx0000000010110000  break 0x200
{                      "break NUM , NUM",{0x0000010D,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx101100xx  break 0, 4
{            "bsel.v WREG , WREG , WREG",{0x78C0001E,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx011110xx  bsel.v $w0, $w0, $w0
{            "bseli.b WREG , WREG , NUM",{0x7A000001,0x00FFFFC0}}, // 01011110xxxxxxxxxxxxxxxx100000xx  bseli.b $w0, $w0, 0
{            "bset.b WREG , WREG , WREG",{0x7A00000D,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx101100xx  bset.b $w0, $w0, $w0
{            "bset.d WREG , WREG , WREG",{0x7A60000D,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx101100xx  bset.d $w0, $w0, $w0
{            "bset.h WREG , WREG , WREG",{0x7A20000D,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx101100xx  bset.h $w0, $w0, $w0
{            "bset.w WREG , WREG , WREG",{0x7A40000D,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx101100xx  bset.w $w0, $w0, $w0
{            "bseti.b WREG , WREG , NUM",{0x7A700009,0x0007FFC0}}, // 01011110xxx01110xxxxxxxx100100xx  bseti.b $w0, $w0, 0
{            "bseti.d WREG , WREG , NUM",{0x7A000009,0x003FFFC0}}, // 01011110xxxxxx00xxxxxxxx100100xx  bseti.d $w0, $w0, 0
{            "bseti.h WREG , WREG , NUM",{0x7A600009,0x000FFFC0}}, // 01011110xxxx0110xxxxxxxx100100xx  bseti.h $w0, $w0, 0
{            "bseti.w WREG , WREG , NUM",{0x7A400009,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx100100xx  bseti.w $w0, $w0, 0
{                      "bz.b WREG , NUM",{0x47000000,0x001FFFFF}}, // 11100010xxxxx000xxxxxxxxxxxxxxxx  bz.b $w0, 4
{                      "bz.d WREG , NUM",{0x47600000,0x001FFFFF}}, // 11100010xxxxx110xxxxxxxxxxxxxxxx  bz.d $w0, 4
{                      "bz.h WREG , NUM",{0x47200000,0x001FFFFF}}, // 11100010xxxxx100xxxxxxxxxxxxxxxx  bz.h $w0, 4
{                      "bz.v WREG , NUM",{0x45600000,0x001FFFFF}}, // 10100010xxxxx110xxxxxxxxxxxxxxxx  bz.v $w0, 4
{                      "bz.w WREG , NUM",{0x47400000,0x001FFFFF}}, // 11100010xxxxx010xxxxxxxxxxxxxxxx  bz.w $w0, 4
{                          "cache , ( )",{0x7C000025,0x00000000}}, // 00111110000000000000000010100100  cache 0x450, ()
{                      "cache , ( NUM )",{0x7E000025,0x03E0FF80}}, // xx11111000000xxxxxxxxxxx1010010x  cache 0x450, (0x100000)
{                      "cache , NUM ( )",{0x7C010025,0x001F0000}}, // 00111110xxxxx0000000000010100100  cache 0x450, 1()
{                  "cache , NUM ( NUM )",{0x7E010025,0x03FFFF80}}, // xx111110xxxxxxxxxxxxxxxx1010010x  cache 0x450, 1(0x100000)
{                 "ceil.w.d FREG , FREG",{0x4620000E,0x0000F7C0}}, // 0110001000000100xxx0xxxx011100xx  ceil.w.d $f0, $f0
{                 "ceil.w.s FREG , FREG",{0x4600000E,0x0000FFC0}}, // 0110001000000000xxxxxxxx011100xx  ceil.w.s $f0, $f0
{             "ceq.b WREG , WREG , WREG",{0x7800000F,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx111100xx  ceq.b $w0, $w0, $w0
{             "ceq.d WREG , WREG , WREG",{0x7860000F,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx111100xx  ceq.d $w0, $w0, $w0
{             "ceq.h WREG , WREG , WREG",{0x7820000F,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx111100xx  ceq.h $w0, $w0, $w0
{             "ceq.w WREG , WREG , WREG",{0x7840000F,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx111100xx  ceq.w $w0, $w0, $w0
{             "ceqi.b WREG , WREG , NUM",{0x78000007,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx111000xx  ceqi.b $w0, $w0, 0
{             "ceqi.d WREG , WREG , NUM",{0x78600007,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx111000xx  ceqi.d $w0, $w0, 0
{             "ceqi.h WREG , WREG , NUM",{0x78200007,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx111000xx  ceqi.h $w0, $w0, 0
{             "ceqi.w WREG , WREG , NUM",{0x78400007,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx111000xx  ceqi.w $w0, $w0, 0
{                    "cfc1 GPREG , CASH",{0x44400000,0x001FF800}}, // 00100010xxxxx010000xxxxx00000000  cfc1 $zero, $0
{                  "cfcmsa GPREG , CASH",{0x787E0019,0x00003FC0}}, // 0001111001111110xxxxxx00100110xx  cfcmsa $zero, $0
{                  "class.d FREG , FREG",{0x4620001B,0x0000FFC0}}, // 0110001000000100xxxxxxxx110110xx  class.d $f0, $f0
{                  "class.s FREG , FREG",{0x4600001B,0x0000FFC0}}, // 0110001000000000xxxxxxxx110110xx  class.s $f0, $f0
{           "cle_s.b WREG , WREG , WREG",{0x7A00000F,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx111100xx  cle_s.b $w0, $w0, $w0
{           "cle_s.d WREG , WREG , WREG",{0x7A60000F,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx111100xx  cle_s.d $w0, $w0, $w0
{           "cle_s.h WREG , WREG , WREG",{0x7A20000F,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx111100xx  cle_s.h $w0, $w0, $w0
{           "cle_s.w WREG , WREG , WREG",{0x7A40000F,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx111100xx  cle_s.w $w0, $w0, $w0
{           "cle_u.b WREG , WREG , WREG",{0x7A80000F,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx111100xx  cle_u.b $w0, $w0, $w0
{           "cle_u.d WREG , WREG , WREG",{0x7AE0000F,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx111100xx  cle_u.d $w0, $w0, $w0
{           "cle_u.h WREG , WREG , WREG",{0x7AA0000F,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx111100xx  cle_u.h $w0, $w0, $w0
{           "cle_u.w WREG , WREG , WREG",{0x7AC0000F,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx111100xx  cle_u.w $w0, $w0, $w0
{           "clei_s.b WREG , WREG , NUM",{0x7A000007,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx111000xx  clei_s.b $w0, $w0, 0
{           "clei_s.d WREG , WREG , NUM",{0x7A600007,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx111000xx  clei_s.d $w0, $w0, 0
{           "clei_s.h WREG , WREG , NUM",{0x7A200007,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx111000xx  clei_s.h $w0, $w0, 0
{           "clei_s.w WREG , WREG , NUM",{0x7A400007,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx111000xx  clei_s.w $w0, $w0, 0
{           "clei_u.b WREG , WREG , NUM",{0x7A800007,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx111000xx  clei_u.b $w0, $w0, 0
{           "clei_u.d WREG , WREG , NUM",{0x7AE00007,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx111000xx  clei_u.d $w0, $w0, 0
{           "clei_u.h WREG , WREG , NUM",{0x7AA00007,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx111000xx  clei_u.h $w0, $w0, 0
{           "clei_u.w WREG , WREG , NUM",{0x7AC00007,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx111000xx  clei_u.w $w0, $w0, 0
{                    "clo GPREG , GPREG",{0x00000051,0x03E0F800}}, // xx00000000000xxx000xxxxx10001010  clo $zero, $zero
{           "clt_s.b WREG , WREG , WREG",{0x7900000F,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx111100xx  clt_s.b $w0, $w0, $w0
{           "clt_s.d WREG , WREG , WREG",{0x7960000F,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx111100xx  clt_s.d $w0, $w0, $w0
{           "clt_s.h WREG , WREG , WREG",{0x7920000F,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx111100xx  clt_s.h $w0, $w0, $w0
{           "clt_s.w WREG , WREG , WREG",{0x7940000F,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx111100xx  clt_s.w $w0, $w0, $w0
{           "clt_u.b WREG , WREG , WREG",{0x7980000F,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx111100xx  clt_u.b $w0, $w0, $w0
{           "clt_u.d WREG , WREG , WREG",{0x79E0000F,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx111100xx  clt_u.d $w0, $w0, $w0
{           "clt_u.h WREG , WREG , WREG",{0x79A0000F,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx111100xx  clt_u.h $w0, $w0, $w0
{           "clt_u.w WREG , WREG , WREG",{0x79C0000F,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx111100xx  clt_u.w $w0, $w0, $w0
{           "clti_s.b WREG , WREG , NUM",{0x79000007,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx111000xx  clti_s.b $w0, $w0, 0
{           "clti_s.d WREG , WREG , NUM",{0x79600007,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx111000xx  clti_s.d $w0, $w0, 0
{           "clti_s.h WREG , WREG , NUM",{0x79200007,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx111000xx  clti_s.h $w0, $w0, 0
{           "clti_s.w WREG , WREG , NUM",{0x79400007,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx111000xx  clti_s.w $w0, $w0, 0
{           "clti_u.b WREG , WREG , NUM",{0x79800007,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx111000xx  clti_u.b $w0, $w0, 0
{           "clti_u.d WREG , WREG , NUM",{0x79E00007,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx111000xx  clti_u.d $w0, $w0, 0
{           "clti_u.h WREG , WREG , NUM",{0x79A00007,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx111000xx  clti_u.h $w0, $w0, 0
{           "clti_u.w WREG , WREG , NUM",{0x79C00007,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx111000xx  clti_u.w $w0, $w0, 0
{                    "clz GPREG , GPREG",{0x00000050,0x03E0F800}}, // xx00000000000xxx000xxxxx00001010  clz $zero, $zero
{          "cmp.af.d FREG , FREG , FREG",{0x46A00000,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx000000xx  cmp.af.d $f0, $f0, $f0
{          "cmp.af.s FREG , FREG , FREG",{0x46800000,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx000000xx  cmp.af.s $f0, $f0, $f0
{          "cmp.eq.d FREG , FREG , FREG",{0x46A00002,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx010000xx  cmp.eq.d $f0, $f0, $f0
{              "cmp.eq.ph GPREG , GPREG",{0x7C000211,0x03FF0000}}, // xx111110xxxxxxxx0100000010001000  cmp.eq.ph $zero, $zero
{          "cmp.eq.s FREG , FREG , FREG",{0x46800002,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx010000xx  cmp.eq.s $f0, $f0, $f0
{          "cmp.le.d FREG , FREG , FREG",{0x46A00006,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx011000xx  cmp.le.d $f0, $f0, $f0
{              "cmp.le.ph GPREG , GPREG",{0x7C000291,0x03FF0000}}, // xx111110xxxxxxxx0100000010001001  cmp.le.ph $zero, $zero
{          "cmp.le.s FREG , FREG , FREG",{0x46800006,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx011000xx  cmp.le.s $f0, $f0, $f0
{          "cmp.lt.d FREG , FREG , FREG",{0x46A00004,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx001000xx  cmp.lt.d $f0, $f0, $f0
{              "cmp.lt.ph GPREG , GPREG",{0x7C000251,0x03FF0000}}, // xx111110xxxxxxxx0100000010001010  cmp.lt.ph $zero, $zero
{          "cmp.lt.s FREG , FREG , FREG",{0x46800004,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx001000xx  cmp.lt.s $f0, $f0, $f0
{         "cmp.saf.d FREG , FREG , FREG",{0x46A00008,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx000100xx  cmp.saf.d $f0, $f0, $f0
{         "cmp.saf.s FREG , FREG , FREG",{0x46800008,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx000100xx  cmp.saf.s $f0, $f0, $f0
{         "cmp.seq.d FREG , FREG , FREG",{0x46A0000A,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx010100xx  cmp.seq.d $f0, $f0, $f0
{         "cmp.seq.s FREG , FREG , FREG",{0x4680000A,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx010100xx  cmp.seq.s $f0, $f0, $f0
{         "cmp.sle.d FREG , FREG , FREG",{0x46A0000E,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx011100xx  cmp.sle.d $f0, $f0, $f0
{         "cmp.sle.s FREG , FREG , FREG",{0x4680000E,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx011100xx  cmp.sle.s $f0, $f0, $f0
{         "cmp.slt.d FREG , FREG , FREG",{0x46A0000C,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx001100xx  cmp.slt.d $f0, $f0, $f0
{         "cmp.slt.s FREG , FREG , FREG",{0x4680000C,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx001100xx  cmp.slt.s $f0, $f0, $f0
{        "cmp.sueq.d FREG , FREG , FREG",{0x46A0000B,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx110100xx  cmp.sueq.d $f0, $f0, $f0
{        "cmp.sueq.s FREG , FREG , FREG",{0x4680000B,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx110100xx  cmp.sueq.s $f0, $f0, $f0
{        "cmp.sule.d FREG , FREG , FREG",{0x46A0000F,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx111100xx  cmp.sule.d $f0, $f0, $f0
{        "cmp.sule.s FREG , FREG , FREG",{0x4680000F,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx111100xx  cmp.sule.s $f0, $f0, $f0
{        "cmp.sult.d FREG , FREG , FREG",{0x46A0000D,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx101100xx  cmp.sult.d $f0, $f0, $f0
{        "cmp.sult.s FREG , FREG , FREG",{0x4680000D,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx101100xx  cmp.sult.s $f0, $f0, $f0
{         "cmp.sun.d FREG , FREG , FREG",{0x46A00009,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx100100xx  cmp.sun.d $f0, $f0, $f0
{         "cmp.sun.s FREG , FREG , FREG",{0x46800009,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx100100xx  cmp.sun.s $f0, $f0, $f0
{         "cmp.ueq.d FREG , FREG , FREG",{0x46A00003,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx110000xx  cmp.ueq.d $f0, $f0, $f0
{         "cmp.ueq.s FREG , FREG , FREG",{0x46800003,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx110000xx  cmp.ueq.s $f0, $f0, $f0
{         "cmp.ule.d FREG , FREG , FREG",{0x46A00007,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx111000xx  cmp.ule.d $f0, $f0, $f0
{         "cmp.ule.s FREG , FREG , FREG",{0x46800007,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx111000xx  cmp.ule.s $f0, $f0, $f0
{         "cmp.ult.d FREG , FREG , FREG",{0x46A00005,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx101000xx  cmp.ult.d $f0, $f0, $f0
{         "cmp.ult.s FREG , FREG , FREG",{0x46800005,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx101000xx  cmp.ult.s $f0, $f0, $f0
{          "cmp.un.d FREG , FREG , FREG",{0x46A00001,0x001FFFC0}}, // 01100010xxxxx101xxxxxxxx100000xx  cmp.un.d $f0, $f0, $f0
{          "cmp.un.s FREG , FREG , FREG",{0x46800001,0x001FFFC0}}, // 01100010xxxxx001xxxxxxxx100000xx  cmp.un.s $f0, $f0, $f0
{   "cmpgdu.eq.qb GPREG , GPREG , GPREG",{0x7C000611,0x03FFF800}}, // xx111110xxxxxxxx011xxxxx10001000  cmpgdu.eq.qb $zero, $zero, $zero
{   "cmpgdu.le.qb GPREG , GPREG , GPREG",{0x7C000691,0x03FFF800}}, // xx111110xxxxxxxx011xxxxx10001001  cmpgdu.le.qb $zero, $zero, $zero
{   "cmpgdu.lt.qb GPREG , GPREG , GPREG",{0x7C000651,0x03FFF800}}, // xx111110xxxxxxxx011xxxxx10001010  cmpgdu.lt.qb $zero, $zero, $zero
{    "cmpgu.eq.qb GPREG , GPREG , GPREG",{0x7C000111,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx10001000  cmpgu.eq.qb $zero, $zero, $zero
{    "cmpgu.le.qb GPREG , GPREG , GPREG",{0x7C000191,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx10001001  cmpgu.le.qb $zero, $zero, $zero
{    "cmpgu.lt.qb GPREG , GPREG , GPREG",{0x7C000151,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx10001010  cmpgu.lt.qb $zero, $zero, $zero
{             "cmpu.eq.qb GPREG , GPREG",{0x7C000011,0x03FF0000}}, // xx111110xxxxxxxx0000000010001000  cmpu.eq.qb $zero, $zero
{             "cmpu.le.qb GPREG , GPREG",{0x7C000091,0x03FF0000}}, // xx111110xxxxxxxx0000000010001001  cmpu.le.qb $zero, $zero
{             "cmpu.lt.qb GPREG , GPREG",{0x7C000051,0x03FF0000}}, // xx111110xxxxxxxx0000000010001010  cmpu.lt.qb $zero, $zero
{        "copy_s.b GPREG , WREG [ NUM ]",{0x78800019,0x000FFFC0}}, // 00011110xxxx0001xxxxxxxx100110xx  copy_s.b $zero, $w0[0]
{        "copy_s.d GPREG , WREG [ NUM ]",{0x78B80019,0x0001FFC0}}, // 00011110x0011101xxxxxxxx100110xx  copy_s.d $zero, $w0[0]
{        "copy_s.h GPREG , WREG [ NUM ]",{0x78A00019,0x0007FFC0}}, // 00011110xxx00101xxxxxxxx100110xx  copy_s.h $zero, $w0[0]
{        "copy_s.w GPREG , WREG [ NUM ]",{0x78B00019,0x0003FFC0}}, // 00011110xx001101xxxxxxxx100110xx  copy_s.w $zero, $w0[0]
{        "copy_u.b GPREG , WREG [ NUM ]",{0x78C00019,0x000FFFC0}}, // 00011110xxxx0011xxxxxxxx100110xx  copy_u.b $zero, $w0[0]
{        "copy_u.d GPREG , WREG [ NUM ]",{0x78F80019,0x0001FFC0}}, // 00011110x0011111xxxxxxxx100110xx  copy_u.d $zero, $w0[0]
{        "copy_u.h GPREG , WREG [ NUM ]",{0x78E00019,0x0007FFC0}}, // 00011110xxx00111xxxxxxxx100110xx  copy_u.h $zero, $w0[0]
{        "copy_u.w GPREG , WREG [ NUM ]",{0x78F00019,0x0003FFC0}}, // 00011110xx001111xxxxxxxx100110xx  copy_u.w $zero, $w0[0]
{                    "ctc1 GPREG , CASH",{0x44C00000,0x001FF800}}, // 00100010xxxxx011000xxxxx00000000  ctc1 $zero, $0
{                  "ctcmsa CASH , GPREG",{0x783E0019,0x0000F9C0}}, // 0001111001111100x00xxxxx100110xx  ctcmsa $0, $zero
{                  "cvt.d.s FREG , FREG",{0x46000021,0x0000FF80}}, // 0110001000000000xxxxxxxx1000010x  cvt.d.s $f0, $f0
{                  "cvt.d.w FREG , FREG",{0x46800021,0x0000FF80}}, // 0110001000000001xxxxxxxx1000010x  cvt.d.w $f0, $f0
{                  "cvt.l.d FREG , FREG",{0x46200025,0x0000FFC0}}, // 0110001000000100xxxxxxxx101001xx  cvt.l.d $f0, $f0
{                  "cvt.l.s FREG , FREG",{0x46000025,0x0000FFC0}}, // 0110001000000000xxxxxxxx101001xx  cvt.l.s $f0, $f0
{                  "cvt.s.d FREG , FREG",{0x46200020,0x0000F7C0}}, // 0110001000000100xxx0xxxx000001xx  cvt.s.d $f0, $f0
{                  "cvt.s.w FREG , FREG",{0x46800020,0x0000FFC0}}, // 0110001000000001xxxxxxxx000001xx  cvt.s.w $f0, $f0
{                  "cvt.w.d FREG , FREG",{0x46200024,0x0000F7C0}}, // 0110001000000100xxx0xxxx001001xx  cvt.w.d $f0, $f0
{                  "cvt.w.s FREG , FREG",{0x46000024,0x0000FFC0}}, // 0110001000000000xxxxxxxx001001xx  cvt.w.s $f0, $f0
{                                "deret",{0x4200001F,0x00000000}}, // 01000010000000000000000011111000  deret
{                                   "di",{0x41606000,0x00000000}}, // 10000010000001100000011000000000  di
{                             "di GPREG",{0x41616000,0x001F0000}}, // 10000010xxxxx1100000011000000000  di $at
{            "div GPREG , GPREG , GPREG",{0x0000009A,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01011001  div $zero, $zero, $zero
{             "div.d FREG , FREG , FREG",{0x46200003,0x001EF780}}, // 011000100xxxx100xxx0xxxx1100000x  div.d $f0, $f0, $f0
{             "div.s FREG , FREG , FREG",{0x46000003,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx110000xx  div.s $f0, $f0, $f0
{           "div_s.b WREG , WREG , WREG",{0x7A000012,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx010010xx  div_s.b $w0, $w0, $w0
{           "div_s.d WREG , WREG , WREG",{0x7A600012,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx010010xx  div_s.d $w0, $w0, $w0
{           "div_s.h WREG , WREG , WREG",{0x7A200012,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx010010xx  div_s.h $w0, $w0, $w0
{           "div_s.w WREG , WREG , WREG",{0x7A400012,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx010010xx  div_s.w $w0, $w0, $w0
{           "div_u.b WREG , WREG , WREG",{0x7A800012,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx010010xx  div_u.b $w0, $w0, $w0
{           "div_u.d WREG , WREG , WREG",{0x7AE00012,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx010010xx  div_u.d $w0, $w0, $w0
{           "div_u.h WREG , WREG , WREG",{0x7AA00012,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx010010xx  div_u.h $w0, $w0, $w0
{           "div_u.w WREG , WREG , WREG",{0x7AC00012,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx010010xx  div_u.w $w0, $w0, $w0
{           "divu GPREG , GPREG , GPREG",{0x0000009B,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11011001  divu $zero, $zero, $zero
{     "dlsa GPREG , GPREG , GPREG , NUM",{0x00000015,0x03FFF8C0}}, // xx000000xxxxxxxx000xxxxx101010xx  dlsa $zero, $zero, $zero, 1
{                   "dmfc1 GPREG , FREG",{0x44200000,0x001FF800}}, // 00100010xxxxx100000xxxxx00000000  dmfc1 $zero, $f0
{                   "dmtc1 GPREG , FREG",{0x44A00000,0x001FF800}}, // 00100010xxxxx101000xxxxx00000000  dmtc1 $zero, $f0
{          "dotp_s.d WREG , WREG , WREG",{0x78600013,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx110010xx  dotp_s.d $w0, $w0, $w0
{          "dotp_s.h WREG , WREG , WREG",{0x78200013,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx110010xx  dotp_s.h $w0, $w0, $w0
{          "dotp_s.w WREG , WREG , WREG",{0x78400013,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx110010xx  dotp_s.w $w0, $w0, $w0
{          "dotp_u.d WREG , WREG , WREG",{0x78E00013,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx110010xx  dotp_u.d $w0, $w0, $w0
{          "dotp_u.h WREG , WREG , WREG",{0x78A00013,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx110010xx  dotp_u.h $w0, $w0, $w0
{          "dotp_u.w WREG , WREG , WREG",{0x78C00013,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx110010xx  dotp_u.w $w0, $w0, $w0
{       "dpa.w.ph ACREG , GPREG , GPREG",{0x7C000030,0x03FF1800}}, // xx111110xxxxxxxx000xx00000001100  dpa.w.ph $ac0, $zero, $zero
{         "dpadd_s.d WREG , WREG , WREG",{0x79600013,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx110010xx  dpadd_s.d $w0, $w0, $w0
{         "dpadd_s.h WREG , WREG , WREG",{0x79200013,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx110010xx  dpadd_s.h $w0, $w0, $w0
{         "dpadd_s.w WREG , WREG , WREG",{0x79400013,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx110010xx  dpadd_s.w $w0, $w0, $w0
{         "dpadd_u.d WREG , WREG , WREG",{0x79E00013,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx110010xx  dpadd_u.d $w0, $w0, $w0
{         "dpadd_u.h WREG , WREG , WREG",{0x79A00013,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx110010xx  dpadd_u.h $w0, $w0, $w0
{         "dpadd_u.w WREG , WREG , WREG",{0x79C00013,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx110010xx  dpadd_u.w $w0, $w0, $w0
{    "dpaq_s.w.ph ACREG , GPREG , GPREG",{0x7C000130,0x03FF1800}}, // xx111110xxxxxxxx100xx00000001100  dpaq_s.w.ph $ac0, $zero, $zero
{    "dpaq_sa.l.w ACREG , GPREG , GPREG",{0x7C000330,0x03FF1800}}, // xx111110xxxxxxxx110xx00000001100  dpaq_sa.l.w $ac0, $zero, $zero
{   "dpaqx_s.w.ph ACREG , GPREG , GPREG",{0x7C000630,0x03FF1800}}, // xx111110xxxxxxxx011xx00000001100  dpaqx_s.w.ph $ac0, $zero, $zero
{  "dpaqx_sa.w.ph ACREG , GPREG , GPREG",{0x7C0006B0,0x03FF1800}}, // xx111110xxxxxxxx011xx00000001101  dpaqx_sa.w.ph $ac0, $zero, $zero
{     "dpau.h.qbl ACREG , GPREG , GPREG",{0x7C0000F0,0x03FF1800}}, // xx111110xxxxxxxx000xx00000001111  dpau.h.qbl $ac0, $zero, $zero
{     "dpau.h.qbr ACREG , GPREG , GPREG",{0x7C0001F0,0x03FF1800}}, // xx111110xxxxxxxx100xx00000001111  dpau.h.qbr $ac0, $zero, $zero
{      "dpax.w.ph ACREG , GPREG , GPREG",{0x7C000230,0x03FF1800}}, // xx111110xxxxxxxx010xx00000001100  dpax.w.ph $ac0, $zero, $zero
{       "dps.w.ph ACREG , GPREG , GPREG",{0x7C000070,0x03FF1800}}, // xx111110xxxxxxxx000xx00000001110  dps.w.ph $ac0, $zero, $zero
{    "dpsq_s.w.ph ACREG , GPREG , GPREG",{0x7C000170,0x03FF1800}}, // xx111110xxxxxxxx100xx00000001110  dpsq_s.w.ph $ac0, $zero, $zero
{    "dpsq_sa.l.w ACREG , GPREG , GPREG",{0x7C000370,0x03FF1800}}, // xx111110xxxxxxxx110xx00000001110  dpsq_sa.l.w $ac0, $zero, $zero
{   "dpsqx_s.w.ph ACREG , GPREG , GPREG",{0x7C000670,0x03FF1800}}, // xx111110xxxxxxxx011xx00000001110  dpsqx_s.w.ph $ac0, $zero, $zero
{  "dpsqx_sa.w.ph ACREG , GPREG , GPREG",{0x7C0006F0,0x03FF1800}}, // xx111110xxxxxxxx011xx00000001111  dpsqx_sa.w.ph $ac0, $zero, $zero
{     "dpsu.h.qbl ACREG , GPREG , GPREG",{0x7C0002F0,0x03FF1800}}, // xx111110xxxxxxxx010xx00000001111  dpsu.h.qbl $ac0, $zero, $zero
{     "dpsu.h.qbr ACREG , GPREG , GPREG",{0x7C0003F0,0x03FF1800}}, // xx111110xxxxxxxx110xx00000001111  dpsu.h.qbr $ac0, $zero, $zero
{         "dpsub_s.d WREG , WREG , WREG",{0x7A600013,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx110010xx  dpsub_s.d $w0, $w0, $w0
{         "dpsub_s.h WREG , WREG , WREG",{0x7A200013,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx110010xx  dpsub_s.h $w0, $w0, $w0
{         "dpsub_s.w WREG , WREG , WREG",{0x7A400013,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx110010xx  dpsub_s.w $w0, $w0, $w0
{         "dpsub_u.d WREG , WREG , WREG",{0x7AE00013,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx110010xx  dpsub_u.d $w0, $w0, $w0
{         "dpsub_u.h WREG , WREG , WREG",{0x7AA00013,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx110010xx  dpsub_u.h $w0, $w0, $w0
{         "dpsub_u.w WREG , WREG , WREG",{0x7AC00013,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx110010xx  dpsub_u.w $w0, $w0, $w0
{      "dpsx.w.ph ACREG , GPREG , GPREG",{0x7C000270,0x03FF1800}}, // xx111110xxxxxxxx010xx00000001110  dpsx.w.ph $ac0, $zero, $zero
{                                  "ehb",{0x000000C0,0x00000000}}, // 00000000000000000000000000000011  ehb
{                                   "ei",{0x41606020,0x00000000}}, // 10000010000001100000011000000100  ei
{                             "ei GPREG",{0x41616020,0x001F0000}}, // 10000010xxxxx1100000011000000100  ei $at
{                                 "eret",{0x42000018,0x00000000}}, // 01000010000000000000000000011000  eret
{        "ext GPREG , GPREG , NUM , NUM",{0x7C000000,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx000000xx  ext $zero, $zero, 0, 1
{             "extp GPREG , ACREG , NUM",{0x7C0000B8,0x03FF1800}}, // xx111110xxxxxxxx000xx00000011101  extp $zero, $ac0, 0
{           "extpdp GPREG , ACREG , NUM",{0x7C0002B8,0x03FF1800}}, // xx111110xxxxxxxx010xx00000011101  extpdp $zero, $ac0, 0
{        "extpdpv GPREG , ACREG , GPREG",{0x7C0002F8,0x03FF1800}}, // xx111110xxxxxxxx010xx00000011111  extpdpv $zero, $ac0, $zero
{          "extpv GPREG , ACREG , GPREG",{0x7C0000F8,0x03FF1800}}, // xx111110xxxxxxxx000xx00000011111  extpv $zero, $ac0, $zero
{           "extr.w GPREG , ACREG , NUM",{0x7C000038,0x03FF1800}}, // xx111110xxxxxxxx000xx00000011100  extr.w $zero, $ac0, 0
{         "extr_r.w GPREG , ACREG , NUM",{0x7C000138,0x03FF1800}}, // xx111110xxxxxxxx100xx00000011100  extr_r.w $zero, $ac0, 0
{        "extr_rs.w GPREG , ACREG , NUM",{0x7C0001B8,0x03FF1800}}, // xx111110xxxxxxxx100xx00000011101  extr_rs.w $zero, $ac0, 0
{         "extr_s.h GPREG , ACREG , NUM",{0x7C0003B8,0x03FF1800}}, // xx111110xxxxxxxx110xx00000011101  extr_s.h $zero, $ac0, 0
{        "extrv.w GPREG , ACREG , GPREG",{0x7C000078,0x03FF1800}}, // xx111110xxxxxxxx000xx00000011110  extrv.w $zero, $ac0, $zero
{      "extrv_r.w GPREG , ACREG , GPREG",{0x7C000178,0x03FF1800}}, // xx111110xxxxxxxx100xx00000011110  extrv_r.w $zero, $ac0, $zero
{     "extrv_rs.w GPREG , ACREG , GPREG",{0x7C0001F8,0x03FF1800}}, // xx111110xxxxxxxx100xx00000011111  extrv_rs.w $zero, $ac0, $zero
{      "extrv_s.h GPREG , ACREG , GPREG",{0x7C0003F8,0x03FF1800}}, // xx111110xxxxxxxx110xx00000011111  extrv_s.h $zero, $ac0, $zero
{            "fadd.d WREG , WREG , WREG",{0x7820001B,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx110110xx  fadd.d $w0, $w0, $w0
{            "fadd.w WREG , WREG , WREG",{0x7800001B,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx110110xx  fadd.w $w0, $w0, $w0
{            "fcaf.d WREG , WREG , WREG",{0x7820001A,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx010110xx  fcaf.d $w0, $w0, $w0
{            "fcaf.w WREG , WREG , WREG",{0x7800001A,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx010110xx  fcaf.w $w0, $w0, $w0
{            "fceq.d WREG , WREG , WREG",{0x78A0001A,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx010110xx  fceq.d $w0, $w0, $w0
{            "fceq.w WREG , WREG , WREG",{0x7880001A,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx010110xx  fceq.w $w0, $w0, $w0
{                 "fclass.d WREG , WREG",{0x7B21001E,0x0000FFC0}}, // 1101111010000100xxxxxxxx011110xx  fclass.d $w0, $w0
{                 "fclass.w WREG , WREG",{0x7B20001E,0x0000FFC0}}, // 1101111000000100xxxxxxxx011110xx  fclass.w $w0, $w0
{            "fcle.d WREG , WREG , WREG",{0x79A0001A,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx010110xx  fcle.d $w0, $w0, $w0
{            "fcle.w WREG , WREG , WREG",{0x7980001A,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx010110xx  fcle.w $w0, $w0, $w0
{            "fclt.d WREG , WREG , WREG",{0x7920001A,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx010110xx  fclt.d $w0, $w0, $w0
{            "fclt.w WREG , WREG , WREG",{0x7900001A,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx010110xx  fclt.w $w0, $w0, $w0
{            "fcne.d WREG , WREG , WREG",{0x78E0001C,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx001110xx  fcne.d $w0, $w0, $w0
{            "fcne.w WREG , WREG , WREG",{0x78C0001C,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx001110xx  fcne.w $w0, $w0, $w0
{            "fcor.d WREG , WREG , WREG",{0x7860001C,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx001110xx  fcor.d $w0, $w0, $w0
{            "fcor.w WREG , WREG , WREG",{0x7840001C,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx001110xx  fcor.w $w0, $w0, $w0
{           "fcueq.d WREG , WREG , WREG",{0x78E0001A,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx010110xx  fcueq.d $w0, $w0, $w0
{           "fcueq.w WREG , WREG , WREG",{0x78C0001A,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx010110xx  fcueq.w $w0, $w0, $w0
{           "fcule.d WREG , WREG , WREG",{0x79E0001A,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx010110xx  fcule.d $w0, $w0, $w0
{           "fcule.w WREG , WREG , WREG",{0x79C0001A,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx010110xx  fcule.w $w0, $w0, $w0
{           "fcult.d WREG , WREG , WREG",{0x7960001A,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx010110xx  fcult.d $w0, $w0, $w0
{           "fcult.w WREG , WREG , WREG",{0x7940001A,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx010110xx  fcult.w $w0, $w0, $w0
{            "fcun.d WREG , WREG , WREG",{0x7860001A,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx010110xx  fcun.d $w0, $w0, $w0
{            "fcun.w WREG , WREG , WREG",{0x7840001A,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx010110xx  fcun.w $w0, $w0, $w0
{           "fcune.d WREG , WREG , WREG",{0x78A0001C,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx001110xx  fcune.d $w0, $w0, $w0
{           "fcune.w WREG , WREG , WREG",{0x7880001C,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx001110xx  fcune.w $w0, $w0, $w0
{            "fdiv.d WREG , WREG , WREG",{0x78E0001B,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx110110xx  fdiv.d $w0, $w0, $w0
{            "fdiv.w WREG , WREG , WREG",{0x78C0001B,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx110110xx  fdiv.w $w0, $w0, $w0
{           "fexdo.h WREG , WREG , WREG",{0x7A00001B,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx110110xx  fexdo.h $w0, $w0, $w0
{           "fexdo.w WREG , WREG , WREG",{0x7A20001B,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx110110xx  fexdo.w $w0, $w0, $w0
{           "fexp2.d WREG , WREG , WREG",{0x79E0001B,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx110110xx  fexp2.d $w0, $w0, $w0
{           "fexp2.w WREG , WREG , WREG",{0x79C0001B,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx110110xx  fexp2.w $w0, $w0, $w0
{                 "fexupl.d WREG , WREG",{0x7B31001E,0x0000FFC0}}, // 1101111010001100xxxxxxxx011110xx  fexupl.d $w0, $w0
{                 "fexupl.w WREG , WREG",{0x7B30001E,0x0000FFC0}}, // 1101111000001100xxxxxxxx011110xx  fexupl.w $w0, $w0
{                 "fexupr.d WREG , WREG",{0x7B33001E,0x0000FFC0}}, // 1101111011001100xxxxxxxx011110xx  fexupr.d $w0, $w0
{                 "fexupr.w WREG , WREG",{0x7B32001E,0x0000FFC0}}, // 1101111001001100xxxxxxxx011110xx  fexupr.w $w0, $w0
{                "ffint_s.d WREG , WREG",{0x7B3D001E,0x0000FFC0}}, // 1101111010111100xxxxxxxx011110xx  ffint_s.d $w0, $w0
{                "ffint_s.w WREG , WREG",{0x7B3C001E,0x0000FFC0}}, // 1101111000111100xxxxxxxx011110xx  ffint_s.w $w0, $w0
{                "ffint_u.d WREG , WREG",{0x7B3F001E,0x0000FFC0}}, // 1101111011111100xxxxxxxx011110xx  ffint_u.d $w0, $w0
{                "ffint_u.w WREG , WREG",{0x7B3E001E,0x0000FFC0}}, // 1101111001111100xxxxxxxx011110xx  ffint_u.w $w0, $w0
{                   "ffql.d WREG , WREG",{0x7B35001E,0x0000FFC0}}, // 1101111010101100xxxxxxxx011110xx  ffql.d $w0, $w0
{                   "ffql.w WREG , WREG",{0x7B34001E,0x0000FFC0}}, // 1101111000101100xxxxxxxx011110xx  ffql.w $w0, $w0
{                   "ffqr.d WREG , WREG",{0x7B37001E,0x0000FFC0}}, // 1101111011101100xxxxxxxx011110xx  ffqr.d $w0, $w0
{                   "ffqr.w WREG , WREG",{0x7B36001E,0x0000FFC0}}, // 1101111001101100xxxxxxxx011110xx  ffqr.w $w0, $w0
{                  "fill.b WREG , GPREG",{0x7B00001E,0x0000FFC0}}, // 1101111000000000xxxxxxxx011110xx  fill.b $w0, $zero
{                  "fill.d WREG , GPREG",{0x7B03001E,0x0000FFC0}}, // 1101111011000000xxxxxxxx011110xx  fill.d $w0, $zero
{                  "fill.h WREG , GPREG",{0x7B01001E,0x0000FFC0}}, // 1101111010000000xxxxxxxx011110xx  fill.h $w0, $zero
{                  "fill.w WREG , GPREG",{0x7B02001E,0x0000FFC0}}, // 1101111001000000xxxxxxxx011110xx  fill.w $w0, $zero
{                  "flog2.d WREG , WREG",{0x7B2F001E,0x0000FFC0}}, // 1101111011110100xxxxxxxx011110xx  flog2.d $w0, $w0
{                  "flog2.w WREG , WREG",{0x7B2E001E,0x0000FFC0}}, // 1101111001110100xxxxxxxx011110xx  flog2.w $w0, $w0
{                "floor.w.d FREG , FREG",{0x4620000F,0x0000F7C0}}, // 0110001000000100xxx0xxxx111100xx  floor.w.d $f0, $f0
{                "floor.w.s FREG , FREG",{0x4600000F,0x0000FFC0}}, // 0110001000000000xxxxxxxx111100xx  floor.w.s $f0, $f0
{           "fmadd.d WREG , WREG , WREG",{0x7920001B,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx110110xx  fmadd.d $w0, $w0, $w0
{           "fmadd.w WREG , WREG , WREG",{0x7900001B,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx110110xx  fmadd.w $w0, $w0, $w0
{            "fmax.d WREG , WREG , WREG",{0x7BA0001B,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx110110xx  fmax.d $w0, $w0, $w0
{            "fmax.w WREG , WREG , WREG",{0x7B80001B,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx110110xx  fmax.w $w0, $w0, $w0
{          "fmax_a.d WREG , WREG , WREG",{0x7BE0001B,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx110110xx  fmax_a.d $w0, $w0, $w0
{          "fmax_a.w WREG , WREG , WREG",{0x7BC0001B,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx110110xx  fmax_a.w $w0, $w0, $w0
{            "fmin.d WREG , WREG , WREG",{0x7B20001B,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx110110xx  fmin.d $w0, $w0, $w0
{            "fmin.w WREG , WREG , WREG",{0x7B00001B,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx110110xx  fmin.w $w0, $w0, $w0
{          "fmin_a.d WREG , WREG , WREG",{0x7B60001B,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx110110xx  fmin_a.d $w0, $w0, $w0
{          "fmin_a.w WREG , WREG , WREG",{0x7B40001B,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx110110xx  fmin_a.w $w0, $w0, $w0
{           "fmsub.d WREG , WREG , WREG",{0x7960001B,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx110110xx  fmsub.d $w0, $w0, $w0
{           "fmsub.w WREG , WREG , WREG",{0x7940001B,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx110110xx  fmsub.w $w0, $w0, $w0
{            "fmul.d WREG , WREG , WREG",{0x78A0001B,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx110110xx  fmul.d $w0, $w0, $w0
{            "fmul.w WREG , WREG , WREG",{0x7880001B,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx110110xx  fmul.w $w0, $w0, $w0
{                   "frcp.d WREG , WREG",{0x7B2B001E,0x0000FFC0}}, // 1101111011010100xxxxxxxx011110xx  frcp.d $w0, $w0
{                   "frcp.w WREG , WREG",{0x7B2A001E,0x0000FFC0}}, // 1101111001010100xxxxxxxx011110xx  frcp.w $w0, $w0
{                  "frint.d WREG , WREG",{0x7B2D001E,0x0000FFC0}}, // 1101111010110100xxxxxxxx011110xx  frint.d $w0, $w0
{                  "frint.w WREG , WREG",{0x7B2C001E,0x0000FFC0}}, // 1101111000110100xxxxxxxx011110xx  frint.w $w0, $w0
{                 "frsqrt.d WREG , WREG",{0x7B29001E,0x0000FFC0}}, // 1101111010010100xxxxxxxx011110xx  frsqrt.d $w0, $w0
{                 "frsqrt.w WREG , WREG",{0x7B28001E,0x0000FFC0}}, // 1101111000010100xxxxxxxx011110xx  frsqrt.w $w0, $w0
{            "fsaf.d WREG , WREG , WREG",{0x7A20001A,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx010110xx  fsaf.d $w0, $w0, $w0
{            "fsaf.w WREG , WREG , WREG",{0x7A00001A,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx010110xx  fsaf.w $w0, $w0, $w0
{            "fseq.d WREG , WREG , WREG",{0x7AA0001A,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx010110xx  fseq.d $w0, $w0, $w0
{            "fseq.w WREG , WREG , WREG",{0x7A80001A,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx010110xx  fseq.w $w0, $w0, $w0
{            "fsle.d WREG , WREG , WREG",{0x7BA0001A,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx010110xx  fsle.d $w0, $w0, $w0
{            "fsle.w WREG , WREG , WREG",{0x7B80001A,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx010110xx  fsle.w $w0, $w0, $w0
{            "fslt.d WREG , WREG , WREG",{0x7B20001A,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx010110xx  fslt.d $w0, $w0, $w0
{            "fslt.w WREG , WREG , WREG",{0x7B00001A,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx010110xx  fslt.w $w0, $w0, $w0
{            "fsne.d WREG , WREG , WREG",{0x7AE0001C,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx001110xx  fsne.d $w0, $w0, $w0
{            "fsne.w WREG , WREG , WREG",{0x7AC0001C,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx001110xx  fsne.w $w0, $w0, $w0
{            "fsor.d WREG , WREG , WREG",{0x7A60001C,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx001110xx  fsor.d $w0, $w0, $w0
{            "fsor.w WREG , WREG , WREG",{0x7A40001C,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx001110xx  fsor.w $w0, $w0, $w0
{                  "fsqrt.d WREG , WREG",{0x7B27001E,0x0000FFC0}}, // 1101111011100100xxxxxxxx011110xx  fsqrt.d $w0, $w0
{                  "fsqrt.w WREG , WREG",{0x7B26001E,0x0000FFC0}}, // 1101111001100100xxxxxxxx011110xx  fsqrt.w $w0, $w0
{            "fsub.d WREG , WREG , WREG",{0x7860001B,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx110110xx  fsub.d $w0, $w0, $w0
{            "fsub.w WREG , WREG , WREG",{0x7840001B,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx110110xx  fsub.w $w0, $w0, $w0
{           "fsueq.d WREG , WREG , WREG",{0x7AE0001A,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx010110xx  fsueq.d $w0, $w0, $w0
{           "fsueq.w WREG , WREG , WREG",{0x7AC0001A,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx010110xx  fsueq.w $w0, $w0, $w0
{           "fsule.d WREG , WREG , WREG",{0x7BE0001A,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx010110xx  fsule.d $w0, $w0, $w0
{           "fsule.w WREG , WREG , WREG",{0x7BC0001A,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx010110xx  fsule.w $w0, $w0, $w0
{           "fsult.d WREG , WREG , WREG",{0x7B60001A,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx010110xx  fsult.d $w0, $w0, $w0
{           "fsult.w WREG , WREG , WREG",{0x7B40001A,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx010110xx  fsult.w $w0, $w0, $w0
{            "fsun.d WREG , WREG , WREG",{0x7A60001A,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx010110xx  fsun.d $w0, $w0, $w0
{            "fsun.w WREG , WREG , WREG",{0x7A40001A,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx010110xx  fsun.w $w0, $w0, $w0
{           "fsune.d WREG , WREG , WREG",{0x7AA0001C,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx001110xx  fsune.d $w0, $w0, $w0
{           "fsune.w WREG , WREG , WREG",{0x7A80001C,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx001110xx  fsune.w $w0, $w0, $w0
{                "ftint_s.d WREG , WREG",{0x7B39001E,0x0000FFC0}}, // 1101111010011100xxxxxxxx011110xx  ftint_s.d $w0, $w0
{                "ftint_s.w WREG , WREG",{0x7B38001E,0x0000FFC0}}, // 1101111000011100xxxxxxxx011110xx  ftint_s.w $w0, $w0
{                "ftint_u.d WREG , WREG",{0x7B3B001E,0x0000FFC0}}, // 1101111011011100xxxxxxxx011110xx  ftint_u.d $w0, $w0
{                "ftint_u.w WREG , WREG",{0x7B3A001E,0x0000FFC0}}, // 1101111001011100xxxxxxxx011110xx  ftint_u.w $w0, $w0
{             "ftq.h WREG , WREG , WREG",{0x7A80001B,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx110110xx  ftq.h $w0, $w0, $w0
{             "ftq.w WREG , WREG , WREG",{0x7AA0001B,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx110110xx  ftq.w $w0, $w0, $w0
{               "ftrunc_s.d WREG , WREG",{0x7B23001E,0x0000FFC0}}, // 1101111011000100xxxxxxxx011110xx  ftrunc_s.d $w0, $w0
{               "ftrunc_s.w WREG , WREG",{0x7B22001E,0x0000FFC0}}, // 1101111001000100xxxxxxxx011110xx  ftrunc_s.w $w0, $w0
{               "ftrunc_u.d WREG , WREG",{0x7B25001E,0x0000FFC0}}, // 1101111010100100xxxxxxxx011110xx  ftrunc_u.d $w0, $w0
{               "ftrunc_u.w WREG , WREG",{0x7B24001E,0x0000FFC0}}, // 1101111000100100xxxxxxxx011110xx  ftrunc_u.w $w0, $w0
{          "hadd_s.d WREG , WREG , WREG",{0x7A600015,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx101010xx  hadd_s.d $w0, $w0, $w0
{          "hadd_s.h WREG , WREG , WREG",{0x7A200015,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx101010xx  hadd_s.h $w0, $w0, $w0
{          "hadd_s.w WREG , WREG , WREG",{0x7A400015,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx101010xx  hadd_s.w $w0, $w0, $w0
{          "hadd_u.d WREG , WREG , WREG",{0x7AE00015,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx101010xx  hadd_u.d $w0, $w0, $w0
{          "hadd_u.h WREG , WREG , WREG",{0x7AA00015,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx101010xx  hadd_u.h $w0, $w0, $w0
{          "hadd_u.w WREG , WREG , WREG",{0x7AC00015,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx101010xx  hadd_u.w $w0, $w0, $w0
{          "hsub_s.d WREG , WREG , WREG",{0x7B600015,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx101010xx  hsub_s.d $w0, $w0, $w0
{          "hsub_s.h WREG , WREG , WREG",{0x7B200015,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx101010xx  hsub_s.h $w0, $w0, $w0
{          "hsub_s.w WREG , WREG , WREG",{0x7B400015,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx101010xx  hsub_s.w $w0, $w0, $w0
{          "hsub_u.d WREG , WREG , WREG",{0x7BE00015,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx101010xx  hsub_u.d $w0, $w0, $w0
{          "hsub_u.h WREG , WREG , WREG",{0x7BA00015,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx101010xx  hsub_u.h $w0, $w0, $w0
{          "hsub_u.w WREG , WREG , WREG",{0x7BC00015,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx101010xx  hsub_u.w $w0, $w0, $w0
{           "ilvev.b WREG , WREG , WREG",{0x7B000014,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx001010xx  ilvev.b $w0, $w0, $w0
{           "ilvev.d WREG , WREG , WREG",{0x7B600014,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx001010xx  ilvev.d $w0, $w0, $w0
{           "ilvev.h WREG , WREG , WREG",{0x7B200014,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx001010xx  ilvev.h $w0, $w0, $w0
{           "ilvev.w WREG , WREG , WREG",{0x7B400014,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx001010xx  ilvev.w $w0, $w0, $w0
{            "ilvl.b WREG , WREG , WREG",{0x7A000014,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx001010xx  ilvl.b $w0, $w0, $w0
{            "ilvl.d WREG , WREG , WREG",{0x7A600014,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx001010xx  ilvl.d $w0, $w0, $w0
{            "ilvl.h WREG , WREG , WREG",{0x7A200014,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx001010xx  ilvl.h $w0, $w0, $w0
{            "ilvl.w WREG , WREG , WREG",{0x7A400014,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx001010xx  ilvl.w $w0, $w0, $w0
{           "ilvod.b WREG , WREG , WREG",{0x7B800014,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx001010xx  ilvod.b $w0, $w0, $w0
{           "ilvod.d WREG , WREG , WREG",{0x7BE00014,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx001010xx  ilvod.d $w0, $w0, $w0
{           "ilvod.h WREG , WREG , WREG",{0x7BA00014,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx001010xx  ilvod.h $w0, $w0, $w0
{           "ilvod.w WREG , WREG , WREG",{0x7BC00014,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx001010xx  ilvod.w $w0, $w0, $w0
{            "ilvr.b WREG , WREG , WREG",{0x7A800014,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx001010xx  ilvr.b $w0, $w0, $w0
{            "ilvr.d WREG , WREG , WREG",{0x7AE00014,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx001010xx  ilvr.d $w0, $w0, $w0
{            "ilvr.h WREG , WREG , WREG",{0x7AA00014,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx001010xx  ilvr.h $w0, $w0, $w0
{            "ilvr.w WREG , WREG , WREG",{0x7AC00014,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx001010xx  ilvr.w $w0, $w0, $w0
{        "ins GPREG , GPREG , NUM , NUM",{0x7C000004,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx001000xx  ins $zero, $zero, 0, 1
{        "insert.b WREG [ NUM ] , GPREG",{0x79000019,0x000FFFC0}}, // 10011110xxxx0000xxxxxxxx100110xx  insert.b $w0[0], $zero
{        "insert.d WREG [ NUM ] , GPREG",{0x79380019,0x0001FFC0}}, // 10011110x0011100xxxxxxxx100110xx  insert.d $w0[0], $zero
{        "insert.h WREG [ NUM ] , GPREG",{0x79200019,0x0007FFC0}}, // 10011110xxx00100xxxxxxxx100110xx  insert.h $w0[0], $zero
{        "insert.w WREG [ NUM ] , GPREG",{0x79300019,0x0003FFC0}}, // 10011110xx001100xxxxxxxx100110xx  insert.w $w0[0], $zero
{                   "insv GPREG , GPREG",{0x7C00000C,0x03FF0000}}, // xx111110xxxxxxxx0000000000110000  insv $zero, $zero
{  "insve.b WREG [ NUM ] , WREG [ NUM ]",{0x79400019,0x000FFFC0}}, // 10011110xxxx0010xxxxxxxx100110xx  insve.b $w0[0], $w0[0]
{  "insve.d WREG [ NUM ] , WREG [ NUM ]",{0x79780019,0x0001FFC0}}, // 10011110x0011110xxxxxxxx100110xx  insve.d $w0[0], $w0[0]
{  "insve.h WREG [ NUM ] , WREG [ NUM ]",{0x79600019,0x0007FFC0}}, // 10011110xxx00110xxxxxxxx100110xx  insve.h $w0[0], $w0[0]
{  "insve.w WREG [ NUM ] , WREG [ NUM ]",{0x79700019,0x0003FFC0}}, // 10011110xx001110xxxxxxxx100110xx  insve.w $w0[0], $w0[0]
{                                "j NUM",{0x08000000,0x03FFFFFF}}, // xx010000xxxxxxxxxxxxxxxxxxxxxxxx  j 0
{                              "jal NUM",{0x0C000000,0x03FFFFFF}}, // xx110000xxxxxxxxxxxxxxxxxxxxxxxx  jal 0
{                   "jalr GPREG , GPREG",{0x00000809,0x03E0F800}}, // xx00000000000xxx000xxxxx10010000  jalr $at, $zero
{                "jalr.hb GPREG , GPREG",{0x00000C09,0x03E0F800}}, // xx00000000000xxx001xxxxx10010000  jalr.hb $at, $zero
{                          "jialc NUM ,",{0xF8000000,0x001FFFFF}}, // 00011111xxxxx000xxxxxxxxxxxxxxxx  jialc 0,
{                            "jic NUM ,",{0xD8000000,0x001FFFFF}}, // 00011011xxxxx000xxxxxxxxxxxxxxxx  jic 0,
{                             "jr GPREG",{0x00000008,0x03E00001}}, // xx00000000000xxx00000000x0010000  jr $zero
{                          "jr.hb GPREG",{0x00000409,0x03E00000}}, // xx00000000000xxx0010000010010000  jr.hb $zero
{                 "lb GPREG , ( GPREG )",{0x80000000,0x03FF0000}}, // xx000001xxxxxxxx0000000000000000  lb $zero, ($zero)
{             "lb GPREG , NUM ( GPREG )",{0x80000100,0x03FFFFFF}}, // xx000001xxxxxxxxxxxxxxxxxxxxxxxx  lb $zero, 0x100($zero)
{                "lbu GPREG , ( GPREG )",{0x90000000,0x03FF0000}}, // xx001001xxxxxxxx0000000000000000  lbu $zero, ($zero)
{            "lbu GPREG , NUM ( GPREG )",{0x90000100,0x03FFFFFF}}, // xx001001xxxxxxxxxxxxxxxxxxxxxxxx  lbu $zero, 0x100($zero)
{         "lbux GPREG , GPREG ( GPREG )",{0x7C00018A,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx01010001  lbux $zero, $zero($zero)
{                "ld.b WREG , ( GPREG )",{0x78000020,0x0000FFC0}}, // 0001111000000000xxxxxxxx000001xx  ld.b $w0, ($zero)
{            "ld.b WREG , NUM ( GPREG )",{0x7A000020,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx000001xx  ld.b $w0, -0x200($zero)
{                "ld.d WREG , ( GPREG )",{0x78000023,0x0000FFC0}}, // 0001111000000000xxxxxxxx110001xx  ld.d $w0, ($zero)
{            "ld.d WREG , NUM ( GPREG )",{0x7A000023,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx110001xx  ld.d $w0, -0x1000($zero)
{                "ld.h WREG , ( GPREG )",{0x78000021,0x0000FFC0}}, // 0001111000000000xxxxxxxx100001xx  ld.h $w0, ($zero)
{            "ld.h WREG , NUM ( GPREG )",{0x7A000021,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx100001xx  ld.h $w0, -0x400($zero)
{                "ld.w WREG , ( GPREG )",{0x78000022,0x0000FFC0}}, // 0001111000000000xxxxxxxx010001xx  ld.w $w0, ($zero)
{            "ld.w WREG , NUM ( GPREG )",{0x7A000022,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx010001xx  ld.w $w0, -0x800($zero)
{                "ldc1 FREG , ( GPREG )",{0xD4000000,0x03FF0000}}, // xx101011xxxxxxxx0000000000000000  ldc1 $f0, ($zero)
{            "ldc1 FREG , NUM ( GPREG )",{0xD4000100,0x03FFFFFF}}, // xx101011xxxxxxxxxxxxxxxxxxxxxxxx  ldc1 $f0, 0x100($zero)
{                      "ldc2 CASH , ( )",{0x49C00000,0x001F0000}}, // 10010010xxxxx0110000000000000000  ldc2 $0, ()
{                  "ldc2 CASH , ( NUM )",{0x49C00100,0x001FFFFF}}, // 10010010xxxxx011xxxxxxxxxxxxxxxx  ldc2 $0, (0x100)
{                     "ldi.b WREG , NUM",{0x7B000007,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx111000xx  ldi.b $w0, 0
{                     "ldi.d WREG , NUM",{0x7B600007,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx111000xx  ldi.d $w0, 0
{                     "ldi.h WREG , NUM",{0x7B200007,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx111000xx  ldi.h $w0, 0
{                     "ldi.w WREG , NUM",{0x7B400007,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx111000xx  ldi.w $w0, 0
{                 "lh GPREG , ( GPREG )",{0x84000000,0x03FF0000}}, // xx100001xxxxxxxx0000000000000000  lh $zero, ($zero)
{             "lh GPREG , NUM ( GPREG )",{0x84000100,0x03FFFFFF}}, // xx100001xxxxxxxxxxxxxxxxxxxxxxxx  lh $zero, 0x100($zero)
{                "lhu GPREG , ( GPREG )",{0x94000000,0x03FF0000}}, // xx101001xxxxxxxx0000000000000000  lhu $zero, ($zero)
{            "lhu GPREG , NUM ( GPREG )",{0x94000100,0x03FFFFFF}}, // xx101001xxxxxxxxxxxxxxxxxxxxxxxx  lhu $zero, 0x100($zero)
{          "lhx GPREG , GPREG ( GPREG )",{0x7C00010A,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx01010000  lhx $zero, $zero($zero)
{                 "ll GPREG , ( GPREG )",{0x7C000036,0x03FF0040}}, // xx111110xxxxxxxx00000000011011x0  ll $zero, ($zero)
{             "ll GPREG , NUM ( GPREG )",{0x7C000136,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx011011xx  ll $zero, 2($zero)
{                "lld GPREG , ( GPREG )",{0x7C000037,0x03FF0040}}, // xx111110xxxxxxxx00000000111011x0  lld $zero, ($zero)
{            "lld GPREG , NUM ( GPREG )",{0x7C000137,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx111011xx  lld $zero, 2($zero)
{      "lsa GPREG , GPREG , GPREG , NUM",{0x00000005,0x03FFF8C0}}, // xx000000xxxxxxxx000xxxxx101000xx  lsa $zero, $zero, $zero, 0
{                 "lw GPREG , ( GPREG )",{0x8C000000,0x03FF0000}}, // xx110001xxxxxxxx0000000000000000  lw $zero, ($zero)
{             "lw GPREG , NUM ( GPREG )",{0x8C000100,0x03FFFFFF}}, // xx110001xxxxxxxxxxxxxxxxxxxxxxxx  lw $zero, 0x100($zero)
{                "lwc1 FREG , ( GPREG )",{0xC4000000,0x03FF0000}}, // xx100011xxxxxxxx0000000000000000  lwc1 $f0, ($zero)
{            "lwc1 FREG , NUM ( GPREG )",{0xC4000100,0x03FFFFFF}}, // xx100011xxxxxxxxxxxxxxxxxxxxxxxx  lwc1 $f0, 0x100($zero)
{                      "lwc2 CASH , ( )",{0x49400000,0x001F0000}}, // 10010010xxxxx0100000000000000000  lwc2 $0, ()
{                  "lwc2 CASH , ( NUM )",{0x49400100,0x001FFFFF}}, // 10010010xxxxx010xxxxxxxxxxxxxxxx  lwc2 $0, (0x100)
{                     "lwpc GPREG , NUM",{0xEC080000,0x03E7FFFF}}, // xx110111xxx10xxxxxxxxxxxxxxxxxxx  lwpc $zero, 0
{                    "lwupc GPREG , NUM",{0xEC100000,0x03E7FFFF}}, // xx110111xxx01xxxxxxxxxxxxxxxxxxx  lwupc $zero, 0
{          "lwx GPREG , GPREG ( GPREG )",{0x7C00000A,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx01010000  lwx $zero, $zero($zero)
{           "madd ACREG , GPREG , GPREG",{0x70000000,0x03FF1800}}, // xx001110xxxxxxxx000xx00000000000  madd $ac0, $zero, $zero
{          "madd_q.h WREG , WREG , WREG",{0x7940001C,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx001110xx  madd_q.h $w0, $w0, $w0
{          "madd_q.w WREG , WREG , WREG",{0x7960001C,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx001110xx  madd_q.w $w0, $w0, $w0
{           "maddf.d FREG , FREG , FREG",{0x46200018,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx000110xx  maddf.d $f0, $f0, $f0
{           "maddf.s FREG , FREG , FREG",{0x46000018,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx000110xx  maddf.s $f0, $f0, $f0
{         "maddr_q.h WREG , WREG , WREG",{0x7B40001C,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx001110xx  maddr_q.h $w0, $w0, $w0
{         "maddr_q.w WREG , WREG , WREG",{0x7B60001C,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx001110xx  maddr_q.w $w0, $w0, $w0
{          "maddu ACREG , GPREG , GPREG",{0x70000001,0x03FF1800}}, // xx001110xxxxxxxx000xx00010000000  maddu $ac0, $zero, $zero
{           "maddv.b WREG , WREG , WREG",{0x78800012,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx010010xx  maddv.b $w0, $w0, $w0
{           "maddv.d WREG , WREG , WREG",{0x78E00012,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx010010xx  maddv.d $w0, $w0, $w0
{           "maddv.h WREG , WREG , WREG",{0x78A00012,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx010010xx  maddv.h $w0, $w0, $w0
{           "maddv.w WREG , WREG , WREG",{0x78C00012,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx010010xx  maddv.w $w0, $w0, $w0
{    "maq_s.w.phl ACREG , GPREG , GPREG",{0x7C000530,0x03FF1800}}, // xx111110xxxxxxxx101xx00000001100  maq_s.w.phl $ac0, $zero, $zero
{    "maq_s.w.phr ACREG , GPREG , GPREG",{0x7C0005B0,0x03FF1800}}, // xx111110xxxxxxxx101xx00000001101  maq_s.w.phr $ac0, $zero, $zero
{   "maq_sa.w.phl ACREG , GPREG , GPREG",{0x7C000430,0x03FF1800}}, // xx111110xxxxxxxx001xx00000001100  maq_sa.w.phl $ac0, $zero, $zero
{   "maq_sa.w.phr ACREG , GPREG , GPREG",{0x7C0004B0,0x03FF1800}}, // xx111110xxxxxxxx001xx00000001101  maq_sa.w.phr $ac0, $zero, $zero
{             "max.d FREG , FREG , FREG",{0x4620001D,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx101110xx  max.d $f0, $f0, $f0
{             "max.s FREG , FREG , FREG",{0x4600001D,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx101110xx  max.s $f0, $f0, $f0
{           "max_a.b WREG , WREG , WREG",{0x7B00000E,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx011100xx  max_a.b $w0, $w0, $w0
{           "max_a.d WREG , WREG , WREG",{0x7B60000E,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx011100xx  max_a.d $w0, $w0, $w0
{           "max_a.h WREG , WREG , WREG",{0x7B20000E,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx011100xx  max_a.h $w0, $w0, $w0
{           "max_a.w WREG , WREG , WREG",{0x7B40000E,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx011100xx  max_a.w $w0, $w0, $w0
{           "max_s.b WREG , WREG , WREG",{0x7900000E,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx011100xx  max_s.b $w0, $w0, $w0
{           "max_s.d WREG , WREG , WREG",{0x7960000E,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx011100xx  max_s.d $w0, $w0, $w0
{           "max_s.h WREG , WREG , WREG",{0x7920000E,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx011100xx  max_s.h $w0, $w0, $w0
{           "max_s.w WREG , WREG , WREG",{0x7940000E,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx011100xx  max_s.w $w0, $w0, $w0
{           "max_u.b WREG , WREG , WREG",{0x7980000E,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx011100xx  max_u.b $w0, $w0, $w0
{           "max_u.d WREG , WREG , WREG",{0x79E0000E,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx011100xx  max_u.d $w0, $w0, $w0
{           "max_u.h WREG , WREG , WREG",{0x79A0000E,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx011100xx  max_u.h $w0, $w0, $w0
{           "max_u.w WREG , WREG , WREG",{0x79C0000E,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx011100xx  max_u.w $w0, $w0, $w0
{            "maxa.d FREG , FREG , FREG",{0x4620001F,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx111110xx  maxa.d $f0, $f0, $f0
{            "maxa.s FREG , FREG , FREG",{0x4600001F,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx111110xx  maxa.s $f0, $f0, $f0
{           "maxi_s.b WREG , WREG , NUM",{0x79000006,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx011000xx  maxi_s.b $w0, $w0, 0
{           "maxi_s.d WREG , WREG , NUM",{0x79600006,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx011000xx  maxi_s.d $w0, $w0, 0
{           "maxi_s.h WREG , WREG , NUM",{0x79200006,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx011000xx  maxi_s.h $w0, $w0, 0
{           "maxi_s.w WREG , WREG , NUM",{0x79400006,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx011000xx  maxi_s.w $w0, $w0, 0
{           "maxi_u.b WREG , WREG , NUM",{0x79800006,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx011000xx  maxi_u.b $w0, $w0, 0
{           "maxi_u.d WREG , WREG , NUM",{0x79E00006,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx011000xx  maxi_u.d $w0, $w0, 0
{           "maxi_u.h WREG , WREG , NUM",{0x79A00006,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx011000xx  maxi_u.h $w0, $w0, 0
{           "maxi_u.w WREG , WREG , NUM",{0x79C00006,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx011000xx  maxi_u.w $w0, $w0, 0
{             "mfc0 GPREG , GPREG , NUM",{0x40000000,0x001FF807}}, // 00000010xxxxx000000xxxxxxxx00000  mfc0 $zero, $zero, 0
{                    "mfc1 GPREG , FREG",{0x44000000,0x001FF800}}, // 00100010xxxxx000000xxxxx00000000  mfc1 $zero, $f0
{             "mfc2 GPREG , GPREG , NUM",{0x48000000,0x001FF807}}, // 00010010xxxxx000000xxxxxxxx00000  mfc2 $zero, $zero, 0
{                   "mfhc1 GPREG , FREG",{0x44600000,0x001FF000}}, // 00100010xxxxx1100000xxxx00000000  mfhc1 $zero, $f0
{                   "mfhi GPREG , ACREG",{0x00000010,0x0060F800}}, // 0000000000000xx0000xxxxx00001000  mfhi $zero, $ac0
{                   "mflo GPREG , ACREG",{0x00000012,0x0060F800}}, // 0000000000000xx0000xxxxx01001000  mflo $zero, $ac0
{             "min.d FREG , FREG , FREG",{0x4620001C,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx001110xx  min.d $f0, $f0, $f0
{             "min.s FREG , FREG , FREG",{0x4600001C,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx001110xx  min.s $f0, $f0, $f0
{           "min_a.b WREG , WREG , WREG",{0x7B80000E,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx011100xx  min_a.b $w0, $w0, $w0
{           "min_a.d WREG , WREG , WREG",{0x7BE0000E,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx011100xx  min_a.d $w0, $w0, $w0
{           "min_a.h WREG , WREG , WREG",{0x7BA0000E,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx011100xx  min_a.h $w0, $w0, $w0
{           "min_a.w WREG , WREG , WREG",{0x7BC0000E,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx011100xx  min_a.w $w0, $w0, $w0
{           "min_s.b WREG , WREG , WREG",{0x7A00000E,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx011100xx  min_s.b $w0, $w0, $w0
{           "min_s.d WREG , WREG , WREG",{0x7A60000E,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx011100xx  min_s.d $w0, $w0, $w0
{           "min_s.h WREG , WREG , WREG",{0x7A20000E,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx011100xx  min_s.h $w0, $w0, $w0
{           "min_s.w WREG , WREG , WREG",{0x7A40000E,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx011100xx  min_s.w $w0, $w0, $w0
{           "min_u.b WREG , WREG , WREG",{0x7A80000E,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx011100xx  min_u.b $w0, $w0, $w0
{           "min_u.d WREG , WREG , WREG",{0x7AE0000E,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx011100xx  min_u.d $w0, $w0, $w0
{           "min_u.h WREG , WREG , WREG",{0x7AA0000E,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx011100xx  min_u.h $w0, $w0, $w0
{           "min_u.w WREG , WREG , WREG",{0x7AC0000E,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx011100xx  min_u.w $w0, $w0, $w0
{            "mina.d FREG , FREG , FREG",{0x4620001E,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx011110xx  mina.d $f0, $f0, $f0
{            "mina.s FREG , FREG , FREG",{0x4600001E,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx011110xx  mina.s $f0, $f0, $f0
{           "mini_s.b WREG , WREG , NUM",{0x7A000006,0x001FFFC0}}, // 01011110xxxxx000xxxxxxxx011000xx  mini_s.b $w0, $w0, 0
{           "mini_s.d WREG , WREG , NUM",{0x7A600006,0x001FFFC0}}, // 01011110xxxxx110xxxxxxxx011000xx  mini_s.d $w0, $w0, 0
{           "mini_s.h WREG , WREG , NUM",{0x7A200006,0x001FFFC0}}, // 01011110xxxxx100xxxxxxxx011000xx  mini_s.h $w0, $w0, 0
{           "mini_s.w WREG , WREG , NUM",{0x7A400006,0x001FFFC0}}, // 01011110xxxxx010xxxxxxxx011000xx  mini_s.w $w0, $w0, 0
{           "mini_u.b WREG , WREG , NUM",{0x7A800006,0x001FFFC0}}, // 01011110xxxxx001xxxxxxxx011000xx  mini_u.b $w0, $w0, 0
{           "mini_u.d WREG , WREG , NUM",{0x7AE00006,0x001FFFC0}}, // 01011110xxxxx111xxxxxxxx011000xx  mini_u.d $w0, $w0, 0
{           "mini_u.h WREG , WREG , NUM",{0x7AA00006,0x001FFFC0}}, // 01011110xxxxx101xxxxxxxx011000xx  mini_u.h $w0, $w0, 0
{           "mini_u.w WREG , WREG , NUM",{0x7AC00006,0x001FFFC0}}, // 01011110xxxxx011xxxxxxxx011000xx  mini_u.w $w0, $w0, 0
{            "mod GPREG , GPREG , GPREG",{0x000000DA,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01011011  mod $zero, $zero, $zero
{           "mod_s.b WREG , WREG , WREG",{0x7B000012,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx010010xx  mod_s.b $w0, $w0, $w0
{           "mod_s.d WREG , WREG , WREG",{0x7B600012,0x001FFFC0}}, // 11011110xxxxx110xxxxxxxx010010xx  mod_s.d $w0, $w0, $w0
{           "mod_s.h WREG , WREG , WREG",{0x7B200012,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx010010xx  mod_s.h $w0, $w0, $w0
{           "mod_s.w WREG , WREG , WREG",{0x7B400012,0x001FFFC0}}, // 11011110xxxxx010xxxxxxxx010010xx  mod_s.w $w0, $w0, $w0
{           "mod_u.b WREG , WREG , WREG",{0x7B800012,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx010010xx  mod_u.b $w0, $w0, $w0
{           "mod_u.d WREG , WREG , WREG",{0x7BE00012,0x001FFFC0}}, // 11011110xxxxx111xxxxxxxx010010xx  mod_u.d $w0, $w0, $w0
{           "mod_u.h WREG , WREG , WREG",{0x7BA00012,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx010010xx  mod_u.h $w0, $w0, $w0
{           "mod_u.w WREG , WREG , WREG",{0x7BC00012,0x001FFFC0}}, // 11011110xxxxx011xxxxxxxx010010xx  mod_u.w $w0, $w0, $w0
{         "modsub GPREG , GPREG , GPREG",{0x7C000490,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00001001  modsub $zero, $zero, $zero
{           "modu GPREG , GPREG , GPREG",{0x000000DB,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11011011  modu $zero, $zero, $zero
{                    "mov.d FREG , FREG",{0x46200006,0x0000F780}}, // 0110001000000100xxx0xxxx0110000x  mov.d $f0, $f0
{                    "mov.s FREG , FREG",{0x46000006,0x0000FFC0}}, // 0110001000000000xxxxxxxx011000xx  mov.s $f0, $f0
{                   "move GPREG , GPREG",{0x00000021,0x03E0F804}}, // xx00000000000xxx000xxxxx10x00100  move $zero, $zero
{                   "move.v WREG , WREG",{0x78BE0019,0x0000FFC0}}, // 0001111001111101xxxxxxxx100110xx  move.v $w0, $w0
{           "msub ACREG , GPREG , GPREG",{0x70000004,0x03FF1800}}, // xx001110xxxxxxxx000xx00000100000  msub $ac0, $zero, $zero
{          "msub_q.h WREG , WREG , WREG",{0x7980001C,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx001110xx  msub_q.h $w0, $w0, $w0
{          "msub_q.w WREG , WREG , WREG",{0x79A0001C,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx001110xx  msub_q.w $w0, $w0, $w0
{           "msubf.d FREG , FREG , FREG",{0x46200019,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx100110xx  msubf.d $f0, $f0, $f0
{           "msubf.s FREG , FREG , FREG",{0x46000019,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx100110xx  msubf.s $f0, $f0, $f0
{         "msubr_q.h WREG , WREG , WREG",{0x7B80001C,0x001FFFC0}}, // 11011110xxxxx001xxxxxxxx001110xx  msubr_q.h $w0, $w0, $w0
{         "msubr_q.w WREG , WREG , WREG",{0x7BA0001C,0x001FFFC0}}, // 11011110xxxxx101xxxxxxxx001110xx  msubr_q.w $w0, $w0, $w0
{          "msubu ACREG , GPREG , GPREG",{0x70000005,0x03FF1800}}, // xx001110xxxxxxxx000xx00010100000  msubu $ac0, $zero, $zero
{           "msubv.b WREG , WREG , WREG",{0x79000012,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx010010xx  msubv.b $w0, $w0, $w0
{           "msubv.d WREG , WREG , WREG",{0x79600012,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx010010xx  msubv.d $w0, $w0, $w0
{           "msubv.h WREG , WREG , WREG",{0x79200012,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx010010xx  msubv.h $w0, $w0, $w0
{           "msubv.w WREG , WREG , WREG",{0x79400012,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx010010xx  msubv.w $w0, $w0, $w0
{             "mtc0 GPREG , GPREG , NUM",{0x40800000,0x001FF807}}, // 00000010xxxxx001000xxxxxxxx00000  mtc0 $zero, $zero, 0
{                    "mtc1 GPREG , FREG",{0x44800000,0x001FF800}}, // 00100010xxxxx001000xxxxx00000000  mtc1 $zero, $f0
{             "mtc2 GPREG , GPREG , NUM",{0x48800000,0x001FF807}}, // 00010010xxxxx001000xxxxxxxx00000  mtc2 $zero, $zero, 0
{                   "mthc1 GPREG , FREG",{0x44E00000,0x001FF000}}, // 00100010xxxxx1110000xxxx00000000  mthc1 $zero, $f0
{                   "mthi GPREG , ACREG",{0x00000011,0x03E01800}}, // xx00000000000xxx000xx00010001000  mthi $zero, $ac0
{                 "mthlip GPREG , ACREG",{0x7C0007F8,0x03E01800}}, // xx11111000000xxx111xx00000011111  mthlip $zero, $ac0
{                   "mtlo GPREG , ACREG",{0x00000013,0x03E01800}}, // xx00000000000xxx000xx00011001000  mtlo $zero, $ac0
{            "muh GPREG , GPREG , GPREG",{0x000000D8,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx00011011  muh $zero, $zero, $zero
{           "muhu GPREG , GPREG , GPREG",{0x000000D9,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx10011011  muhu $zero, $zero, $zero
{            "mul GPREG , GPREG , GPREG",{0x00000098,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx00011001  mul $zero, $zero, $zero
{             "mul.d FREG , FREG , FREG",{0x46200002,0x001EF780}}, // 011000100xxxx100xxx0xxxx0100000x  mul.d $f0, $f0, $f0
{         "mul.ph GPREG , GPREG , GPREG",{0x7C000318,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00011000  mul.ph $zero, $zero, $zero
{             "mul.s FREG , FREG , FREG",{0x46000002,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx010000xx  mul.s $f0, $f0, $f0
{           "mul_q.h WREG , WREG , WREG",{0x7900001C,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx001110xx  mul_q.h $w0, $w0, $w0
{           "mul_q.w WREG , WREG , WREG",{0x7920001C,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx001110xx  mul_q.w $w0, $w0, $w0
{       "mul_s.ph GPREG , GPREG , GPREG",{0x7C000398,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00011001  mul_s.ph $zero, $zero, $zero
{  "muleq_s.w.phl GPREG , GPREG , GPREG",{0x7C000710,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx00001000  muleq_s.w.phl $zero, $zero, $zero
{  "muleq_s.w.phr GPREG , GPREG , GPREG",{0x7C000750,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx00001010  muleq_s.w.phr $zero, $zero, $zero
{ "muleu_s.ph.qbl GPREG , GPREG , GPREG",{0x7C000190,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx00001001  muleu_s.ph.qbl $zero, $zero, $zero
{ "muleu_s.ph.qbr GPREG , GPREG , GPREG",{0x7C0001D0,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx00001011  muleu_s.ph.qbr $zero, $zero, $zero
{     "mulq_rs.ph GPREG , GPREG , GPREG",{0x7C0007D0,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx00001011  mulq_rs.ph $zero, $zero, $zero
{      "mulq_rs.w GPREG , GPREG , GPREG",{0x7C0005D8,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx00011011  mulq_rs.w $zero, $zero, $zero
{      "mulq_s.ph GPREG , GPREG , GPREG",{0x7C000790,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx00001001  mulq_s.ph $zero, $zero, $zero
{       "mulq_s.w GPREG , GPREG , GPREG",{0x7C000598,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx00011001  mulq_s.w $zero, $zero, $zero
{          "mulr_q.h WREG , WREG , WREG",{0x7B00001C,0x001FFFC0}}, // 11011110xxxxx000xxxxxxxx001110xx  mulr_q.h $w0, $w0, $w0
{          "mulr_q.w WREG , WREG , WREG",{0x7B20001C,0x001FFFC0}}, // 11011110xxxxx100xxxxxxxx001110xx  mulr_q.w $w0, $w0, $w0
{     "mulsa.w.ph ACREG , GPREG , GPREG",{0x7C0000B0,0x03FF1800}}, // xx111110xxxxxxxx000xx00000001101  mulsa.w.ph $ac0, $zero, $zero
{  "mulsaq_s.w.ph ACREG , GPREG , GPREG",{0x7C0001B0,0x03FF1800}}, // xx111110xxxxxxxx100xx00000001101  mulsaq_s.w.ph $ac0, $zero, $zero
{           "mult ACREG , GPREG , GPREG",{0x00000018,0x03FF1800}}, // xx000000xxxxxxxx000xx00000011000  mult $ac0, $zero, $zero
{          "multu ACREG , GPREG , GPREG",{0x00000019,0x03FF1800}}, // xx000000xxxxxxxx000xx00010011000  multu $ac0, $zero, $zero
{           "mulu GPREG , GPREG , GPREG",{0x00000099,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx10011001  mulu $zero, $zero, $zero
{            "mulv.b WREG , WREG , WREG",{0x78000012,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx010010xx  mulv.b $w0, $w0, $w0
{            "mulv.d WREG , WREG , WREG",{0x78600012,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx010010xx  mulv.d $w0, $w0, $w0
{            "mulv.h WREG , WREG , WREG",{0x78200012,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx010010xx  mulv.h $w0, $w0, $w0
{            "mulv.w WREG , WREG , WREG",{0x78400012,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx010010xx  mulv.w $w0, $w0, $w0
{                    "neg GPREG , GPREG",{0x00000022,0x001FF800}}, // 00000000xxxxx000000xxxxx01000100  neg $zero, $zero
{                    "neg.d FREG , FREG",{0x46200007,0x0000F780}}, // 0110001000000100xxx0xxxx1110000x  neg.d $f0, $f0
{                    "neg.s FREG , FREG",{0x46000007,0x0000FFC0}}, // 0110001000000000xxxxxxxx111000xx  neg.s $f0, $f0
{                   "negu GPREG , GPREG",{0x00000023,0x001FF800}}, // 00000000xxxxx000000xxxxx11000100  negu $zero, $zero
{                   "nloc.b WREG , WREG",{0x7B08001E,0x0000FFC0}}, // 1101111000010000xxxxxxxx011110xx  nloc.b $w0, $w0
{                   "nloc.d WREG , WREG",{0x7B0B001E,0x0000FFC0}}, // 1101111011010000xxxxxxxx011110xx  nloc.d $w0, $w0
{                   "nloc.h WREG , WREG",{0x7B09001E,0x0000FFC0}}, // 1101111010010000xxxxxxxx011110xx  nloc.h $w0, $w0
{                   "nloc.w WREG , WREG",{0x7B0A001E,0x0000FFC0}}, // 1101111001010000xxxxxxxx011110xx  nloc.w $w0, $w0
{                   "nlzc.b WREG , WREG",{0x7B0C001E,0x0000FFC0}}, // 1101111000110000xxxxxxxx011110xx  nlzc.b $w0, $w0
{                   "nlzc.d WREG , WREG",{0x7B0F001E,0x0000FFC0}}, // 1101111011110000xxxxxxxx011110xx  nlzc.d $w0, $w0
{                   "nlzc.h WREG , WREG",{0x7B0D001E,0x0000FFC0}}, // 1101111010110000xxxxxxxx011110xx  nlzc.h $w0, $w0
{                   "nlzc.w WREG , WREG",{0x7B0E001E,0x0000FFC0}}, // 1101111001110000xxxxxxxx011110xx  nlzc.w $w0, $w0
{                                  "nop",{0x00000000,0x00000000}}, // 00000000000000000000000000000000  nop
{            "nor GPREG , GPREG , GPREG",{0x00010027,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11100100  nor $zero, $zero, $at
{             "nor.v WREG , WREG , WREG",{0x7840001E,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx011110xx  nor.v $w0, $w0, $w0
{             "nori.b WREG , WREG , NUM",{0x7A000000,0x00FFFFC0}}, // 01011110xxxxxxxxxxxxxxxx000000xx  nori.b $w0, $w0, 0
{                    "not GPREG , GPREG",{0x00000027,0x03E0F800}}, // xx00000000000xxx000xxxxx11100100  not $zero, $zero
{             "or GPREG , GPREG , GPREG",{0x00010025,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx10100100  or $zero, $zero, $at
{              "or.v WREG , WREG , WREG",{0x7820001E,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx011110xx  or.v $w0, $w0, $w0
{              "ori GPREG , GPREG , NUM",{0x34000000,0x03FFFFFF}}, // xx101100xxxxxxxxxxxxxxxxxxxxxxxx  ori $zero, $zero, 0
{              "ori.b WREG , WREG , NUM",{0x79000000,0x00FFFFC0}}, // 10011110xxxxxxxxxxxxxxxx000000xx  ori.b $w0, $w0, 0
{      "packrl.ph GPREG , GPREG , GPREG",{0x7C000391,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx10001001  packrl.ph $zero, $zero, $zero
{                                "pause",{0x00000140,0x00000000}}, // 00000000000000001000000000000010  pause
{           "pckev.b WREG , WREG , WREG",{0x79000014,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx001010xx  pckev.b $w0, $w0, $w0
{           "pckev.d WREG , WREG , WREG",{0x79600014,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx001010xx  pckev.d $w0, $w0, $w0
{           "pckev.h WREG , WREG , WREG",{0x79200014,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx001010xx  pckev.h $w0, $w0, $w0
{           "pckev.w WREG , WREG , WREG",{0x79400014,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx001010xx  pckev.w $w0, $w0, $w0
{           "pckod.b WREG , WREG , WREG",{0x79800014,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx001010xx  pckod.b $w0, $w0, $w0
{           "pckod.d WREG , WREG , WREG",{0x79E00014,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx001010xx  pckod.d $w0, $w0, $w0
{           "pckod.h WREG , WREG , WREG",{0x79A00014,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx001010xx  pckod.h $w0, $w0, $w0
{           "pckod.w WREG , WREG , WREG",{0x79C00014,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx001010xx  pckod.w $w0, $w0, $w0
{                   "pcnt.b WREG , WREG",{0x7B04001E,0x0000FFC0}}, // 1101111000100000xxxxxxxx011110xx  pcnt.b $w0, $w0
{                   "pcnt.d WREG , WREG",{0x7B07001E,0x0000FFC0}}, // 1101111011100000xxxxxxxx011110xx  pcnt.d $w0, $w0
{                   "pcnt.h WREG , WREG",{0x7B05001E,0x0000FFC0}}, // 1101111010100000xxxxxxxx011110xx  pcnt.h $w0, $w0
{                   "pcnt.w WREG , WREG",{0x7B06001E,0x0000FFC0}}, // 1101111001100000xxxxxxxx011110xx  pcnt.w $w0, $w0
{        "pick.ph GPREG , GPREG , GPREG",{0x7C0002D1,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx10001011  pick.ph $zero, $zero, $zero
{        "pick.qb GPREG , GPREG , GPREG",{0x7C0000D1,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx10001011  pick.qb $zero, $zero, $zero
{           "preceq.w.phl GPREG , GPREG",{0x7C000312,0x001FF800}}, // 00111110xxxxx000110xxxxx01001000  preceq.w.phl $zero, $zero
{           "preceq.w.phr GPREG , GPREG",{0x7C000352,0x001FF800}}, // 00111110xxxxx000110xxxxx01001010  preceq.w.phr $zero, $zero
{         "precequ.ph.qbl GPREG , GPREG",{0x7C000112,0x001FF800}}, // 00111110xxxxx000100xxxxx01001000  precequ.ph.qbl $zero, $zero
{        "precequ.ph.qbla GPREG , GPREG",{0x7C000192,0x001FF800}}, // 00111110xxxxx000100xxxxx01001001  precequ.ph.qbla $zero, $zero
{         "precequ.ph.qbr GPREG , GPREG",{0x7C000152,0x001FF800}}, // 00111110xxxxx000100xxxxx01001010  precequ.ph.qbr $zero, $zero
{        "precequ.ph.qbra GPREG , GPREG",{0x7C0001D2,0x001FF800}}, // 00111110xxxxx000100xxxxx01001011  precequ.ph.qbra $zero, $zero
{          "preceu.ph.qbl GPREG , GPREG",{0x7C000712,0x001FF800}}, // 00111110xxxxx000111xxxxx01001000  preceu.ph.qbl $zero, $zero
{         "preceu.ph.qbla GPREG , GPREG",{0x7C000792,0x001FF800}}, // 00111110xxxxx000111xxxxx01001001  preceu.ph.qbla $zero, $zero
{          "preceu.ph.qbr GPREG , GPREG",{0x7C000752,0x001FF800}}, // 00111110xxxxx000111xxxxx01001010  preceu.ph.qbr $zero, $zero
{         "preceu.ph.qbra GPREG , GPREG",{0x7C0007D2,0x001FF800}}, // 00111110xxxxx000111xxxxx01001011  preceu.ph.qbra $zero, $zero
{    "precr.qb.ph GPREG , GPREG , GPREG",{0x7C000351,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx10001010  precr.qb.ph $zero, $zero, $zero
{   "precr_sra.ph.w GPREG , GPREG , NUM",{0x7C000791,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx10001001  precr_sra.ph.w $zero, $zero, 0
{ "precr_sra_r.ph.w GPREG , GPREG , NUM",{0x7C0007D1,0x03FFF800}}, // xx111110xxxxxxxx111xxxxx10001011  precr_sra_r.ph.w $zero, $zero, 0
{    "precrq.ph.w GPREG , GPREG , GPREG",{0x7C000511,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx10001000  precrq.ph.w $zero, $zero, $zero
{   "precrq.qb.ph GPREG , GPREG , GPREG",{0x7C000311,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx10001000  precrq.qb.ph $zero, $zero, $zero
{ "precrq_rs.ph.w GPREG , GPREG , GPREG",{0x7C000551,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx10001010  precrq_rs.ph.w $zero, $zero, $zero
{"precrqu_s.qb.ph GPREG , GPREG , GPREG",{0x7C0003D1,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx10001011  precrqu_s.qb.ph $zero, $zero, $zero
{                           "pref , ( )",{0x7C000035,0x00000000}}, // 00111110000000000000000010101100  pref , ()
{                       "pref , ( NUM )",{0x7E000035,0x03E0FF80}}, // xx11111000000xxxxxxxxxxx1010110x  pref , (0x100000)
{                       "pref , NUM ( )",{0x7C010035,0x001F0000}}, // 00111110xxxxx0000000000010101100  pref , 1()
{                   "pref , NUM ( NUM )",{0x7E010035,0x03FFFF80}}, // xx111110xxxxxxxxxxxxxxxx1010110x  pref , 1(0x100000)
{          "prepend GPREG , GPREG , NUM",{0x7C000071,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx10001110  prepend $zero, $zero, 0
{             "raddu.w.qb GPREG , GPREG",{0x7C000510,0x03E0F800}}, // xx11111000000xxx101xxxxx00001000  raddu.w.qb $zero, $zero
{                    "rddsp GPREG , NUM",{0x7C0004B8,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011101  rddsp $zero, 0
{                   "rdhwr GPREG , CASH",{0x7C00E83B,0x001F0000}}, // 00111110xxxxx0000001011111011100  rdhwr $zero, $29
{                  "repl.ph GPREG , NUM",{0x7C000292,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx01001001  repl.ph $zero, 0
{                  "repl.qb GPREG , NUM",{0x7C000092,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx01001001  repl.qb $zero, 0
{               "replv.ph GPREG , GPREG",{0x7C0002D2,0x001FF800}}, // 00111110xxxxx000010xxxxx01001011  replv.ph $zero, $zero
{               "replv.qb GPREG , GPREG",{0x7C0000D2,0x001FF800}}, // 00111110xxxxx000000xxxxx01001011  replv.qb $zero, $zero
{                   "rint.d FREG , FREG",{0x4620001A,0x0000FFC0}}, // 0110001000000100xxxxxxxx010110xx  rint.d $f0, $f0
{                   "rint.s FREG , FREG",{0x4600001A,0x0000FFC0}}, // 0110001000000000xxxxxxxx010110xx  rint.s $f0, $f0
{             "rotr GPREG , GPREG , NUM",{0x00200002,0x001FFFC0}}, // 00000000xxxxx100xxxxxxxx010000xx  rotr $zero, $zero, 0
{          "rotrv GPREG , GPREG , GPREG",{0x00000046,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01100010  rotrv $zero, $zero, $zero
{                "round.w.d FREG , FREG",{0x4620000C,0x0000F7C0}}, // 0110001000000100xxx0xxxx001100xx  round.w.d $f0, $f0
{                "round.w.s FREG , FREG",{0x4600000C,0x0000FFC0}}, // 0110001000000000xxxxxxxx001100xx  round.w.s $f0, $f0
{            "sat_s.b WREG , WREG , NUM",{0x7870000A,0x0007FFC0}}, // 00011110xxx01110xxxxxxxx010100xx  sat_s.b $w0, $w0, 0
{            "sat_s.d WREG , WREG , NUM",{0x7800000A,0x003FFFC0}}, // 00011110xxxxxx00xxxxxxxx010100xx  sat_s.d $w0, $w0, 0
{            "sat_s.h WREG , WREG , NUM",{0x7860000A,0x000FFFC0}}, // 00011110xxxx0110xxxxxxxx010100xx  sat_s.h $w0, $w0, 0
{            "sat_s.w WREG , WREG , NUM",{0x7840000A,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx010100xx  sat_s.w $w0, $w0, 0
{            "sat_u.b WREG , WREG , NUM",{0x78F0000A,0x0007FFC0}}, // 00011110xxx01111xxxxxxxx010100xx  sat_u.b $w0, $w0, 0
{            "sat_u.d WREG , WREG , NUM",{0x7880000A,0x003FFFC0}}, // 00011110xxxxxx01xxxxxxxx010100xx  sat_u.d $w0, $w0, 0
{            "sat_u.h WREG , WREG , NUM",{0x78E0000A,0x000FFFC0}}, // 00011110xxxx0111xxxxxxxx010100xx  sat_u.h $w0, $w0, 0
{            "sat_u.w WREG , WREG , NUM",{0x78C0000A,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx010100xx  sat_u.w $w0, $w0, 0
{                 "sb GPREG , ( GPREG )",{0xA0000000,0x03FF0000}}, // xx000101xxxxxxxx0000000000000000  sb $zero, ($zero)
{             "sb GPREG , NUM ( GPREG )",{0xA0000100,0x03FFFFFF}}, // xx000101xxxxxxxxxxxxxxxxxxxxxxxx  sb $zero, 0x100($zero)
{                 "sc GPREG , ( GPREG )",{0x7C000026,0x03FF0040}}, // xx111110xxxxxxxx00000000011001x0  sc $zero, ($zero)
{             "sc GPREG , NUM ( GPREG )",{0x7C000126,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx011001xx  sc $zero, 2($zero)
{                "scd GPREG , ( GPREG )",{0x7C000027,0x03FF0040}}, // xx111110xxxxxxxx00000000111001x0  scd $zero, ($zero)
{            "scd GPREG , NUM ( GPREG )",{0x7C000127,0x03FFFFC0}}, // xx111110xxxxxxxxxxxxxxxx111001xx  scd $zero, 2($zero)
{                                "sdbbp",{0x0000000E,0x00000000}}, // 00000000000000000000000001110000  sdbbp
{                            "sdbbp NUM",{0x0200000E,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx011100xx  sdbbp 0x80000
{                "sdc1 FREG , ( GPREG )",{0xF4000000,0x03FF0000}}, // xx101111xxxxxxxx0000000000000000  sdc1 $f0, ($zero)
{            "sdc1 FREG , NUM ( GPREG )",{0xF4000100,0x03FFFFFF}}, // xx101111xxxxxxxxxxxxxxxxxxxxxxxx  sdc1 $f0, 0x100($zero)
{                      "sdc2 CASH , ( )",{0x49E00000,0x001F0000}}, // 10010010xxxxx1110000000000000000  sdc2 $0, ()
{                  "sdc2 CASH , ( NUM )",{0x49E00100,0x001FFFFF}}, // 10010010xxxxx111xxxxxxxxxxxxxxxx  sdc2 $0, (0x100)
{                    "seb GPREG , GPREG",{0x7C000420,0x001FF800}}, // 00111110xxxxx000001xxxxx00000100  seb $zero, $zero
{                    "seh GPREG , GPREG",{0x7C000620,0x001FF800}}, // 00111110xxxxx000011xxxxx00000100  seh $zero, $zero
{             "sel.d FREG , FREG , FREG",{0x46200010,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx000010xx  sel.d $f0, $f0, $f0
{             "sel.s FREG , FREG , FREG",{0x46000010,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx000010xx  sel.s $f0, $f0, $f0
{          "seleqz.d FREG , FREG , FREG",{0x46200014,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx001010xx  seleqz.d $f0, $f0, $f0
{          "seleqz.s FREG , FREG , FREG",{0x46000014,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx001010xx  seleqz.s $f0, $f0, $f0
{          "selnez.d FREG , FREG , FREG",{0x46200017,0x001FFFC0}}, // 01100010xxxxx100xxxxxxxx111010xx  selnez.d $f0, $f0, $f0
{          "selnez.s FREG , FREG , FREG",{0x46000017,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx111010xx  selnez.s $f0, $f0, $f0
{                 "sh GPREG , ( GPREG )",{0xA4000000,0x03FF0000}}, // xx100101xxxxxxxx0000000000000000  sh $zero, ($zero)
{             "sh GPREG , NUM ( GPREG )",{0xA4000100,0x03FFFFFF}}, // xx100101xxxxxxxxxxxxxxxxxxxxxxxx  sh $zero, 0x100($zero)
{              "shf.b WREG , WREG , NUM",{0x78000002,0x00FFFFC0}}, // 00011110xxxxxxxxxxxxxxxx010000xx  shf.b $w0, $w0, 0
{              "shf.h WREG , WREG , NUM",{0x79000002,0x00FFFFC0}}, // 10011110xxxxxxxxxxxxxxxx010000xx  shf.h $w0, $w0, 0
{              "shf.w WREG , WREG , NUM",{0x7A000002,0x00FFFFC0}}, // 01011110xxxxxxxxxxxxxxxx010000xx  shf.w $w0, $w0, 0
{                    "shilo ACREG , NUM",{0x7C0006B8,0x03F01800}}, // xx1111100000xxxx011xx00000011101  shilo $ac0, 0
{                 "shilov ACREG , GPREG",{0x7C0006F8,0x03E01800}}, // xx11111000000xxx011xx00000011111  shilov $ac0, $zero
{          "shll.ph GPREG , GPREG , NUM",{0x7C000213,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx11001000  shll.ph $zero, $zero, 0
{          "shll.qb GPREG , GPREG , NUM",{0x7C000013,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx11001000  shll.qb $zero, $zero, 0
{        "shll_s.ph GPREG , GPREG , NUM",{0x7C000313,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx11001000  shll_s.ph $zero, $zero, 0
{         "shll_s.w GPREG , GPREG , NUM",{0x7C000513,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx11001000  shll_s.w $zero, $zero, 0
{       "shllv.ph GPREG , GPREG , GPREG",{0x7C000293,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx11001001  shllv.ph $zero, $zero, $zero
{       "shllv.qb GPREG , GPREG , GPREG",{0x7C000093,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx11001001  shllv.qb $zero, $zero, $zero
{     "shllv_s.ph GPREG , GPREG , GPREG",{0x7C000393,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx11001001  shllv_s.ph $zero, $zero, $zero
{      "shllv_s.w GPREG , GPREG , GPREG",{0x7C000593,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx11001001  shllv_s.w $zero, $zero, $zero
{          "shra.ph GPREG , GPREG , NUM",{0x7C000253,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx11001010  shra.ph $zero, $zero, 0
{          "shra.qb GPREG , GPREG , NUM",{0x7C000113,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx11001000  shra.qb $zero, $zero, 0
{        "shra_r.ph GPREG , GPREG , NUM",{0x7C000353,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx11001010  shra_r.ph $zero, $zero, 0
{        "shra_r.qb GPREG , GPREG , NUM",{0x7C000153,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx11001010  shra_r.qb $zero, $zero, 0
{         "shra_r.w GPREG , GPREG , NUM",{0x7C000553,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx11001010  shra_r.w $zero, $zero, 0
{       "shrav.ph GPREG , GPREG , GPREG",{0x7C0002D3,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx11001011  shrav.ph $zero, $zero, $zero
{       "shrav.qb GPREG , GPREG , GPREG",{0x7C000193,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx11001001  shrav.qb $zero, $zero, $zero
{     "shrav_r.ph GPREG , GPREG , GPREG",{0x7C0003D3,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx11001011  shrav_r.ph $zero, $zero, $zero
{     "shrav_r.qb GPREG , GPREG , GPREG",{0x7C0001D3,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx11001011  shrav_r.qb $zero, $zero, $zero
{      "shrav_r.w GPREG , GPREG , GPREG",{0x7C0005D3,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx11001011  shrav_r.w $zero, $zero, $zero
{          "shrl.ph GPREG , GPREG , NUM",{0x7C000653,0x03FFF800}}, // xx111110xxxxxxxx011xxxxx11001010  shrl.ph $zero, $zero, 0
{          "shrl.qb GPREG , GPREG , NUM",{0x7C000053,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx11001010  shrl.qb $zero, $zero, 0
{       "shrlv.ph GPREG , GPREG , GPREG",{0x7C0006D3,0x03FFF800}}, // xx111110xxxxxxxx011xxxxx11001011  shrlv.ph $zero, $zero, $zero
{       "shrlv.qb GPREG , GPREG , GPREG",{0x7C0000D3,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx11001011  shrlv.qb $zero, $zero, $zero
{          "sld.b WREG , WREG [ GPREG ]",{0x78000014,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx001010xx  sld.b $w0, $w0[$zero]
{          "sld.d WREG , WREG [ GPREG ]",{0x78600014,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx001010xx  sld.d $w0, $w0[$zero]
{          "sld.h WREG , WREG [ GPREG ]",{0x78200014,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx001010xx  sld.h $w0, $w0[$zero]
{          "sld.w WREG , WREG [ GPREG ]",{0x78400014,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx001010xx  sld.w $w0, $w0[$zero]
{           "sldi.b WREG , WREG [ NUM ]",{0x78000019,0x000FFFC0}}, // 00011110xxxx0000xxxxxxxx100110xx  sldi.b $w0, $w0[0]
{           "sldi.d WREG , WREG [ NUM ]",{0x78380019,0x0001FFC0}}, // 00011110x0011100xxxxxxxx100110xx  sldi.d $w0, $w0[0]
{           "sldi.h WREG , WREG [ NUM ]",{0x78200019,0x0007FFC0}}, // 00011110xxx00100xxxxxxxx100110xx  sldi.h $w0, $w0[0]
{           "sldi.w WREG , WREG [ NUM ]",{0x78300019,0x0003FFC0}}, // 00011110xx001100xxxxxxxx100110xx  sldi.w $w0, $w0[0]
{              "sll GPREG , GPREG , NUM",{0x00010000,0x001FFFC0}}, // 00000000xxxxx000xxxxxxxx000000xx  sll $zero, $at, 0
{             "sll.b WREG , WREG , WREG",{0x7800000D,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx101100xx  sll.b $w0, $w0, $w0
{             "sll.d WREG , WREG , WREG",{0x7860000D,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx101100xx  sll.d $w0, $w0, $w0
{             "sll.h WREG , WREG , WREG",{0x7820000D,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx101100xx  sll.h $w0, $w0, $w0
{             "sll.w WREG , WREG , WREG",{0x7840000D,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx101100xx  sll.w $w0, $w0, $w0
{             "slli.b WREG , WREG , NUM",{0x78700009,0x0007FFC0}}, // 00011110xxx01110xxxxxxxx100100xx  slli.b $w0, $w0, 0
{             "slli.d WREG , WREG , NUM",{0x78000009,0x003FFFC0}}, // 00011110xxxxxx00xxxxxxxx100100xx  slli.d $w0, $w0, 0
{             "slli.h WREG , WREG , NUM",{0x78600009,0x000FFFC0}}, // 00011110xxxx0110xxxxxxxx100100xx  slli.h $w0, $w0, 0
{             "slli.w WREG , WREG , NUM",{0x78400009,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx100100xx  slli.w $w0, $w0, 0
{           "sllv GPREG , GPREG , GPREG",{0x00000004,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx00100000  sllv $zero, $zero, $zero
{            "slt GPREG , GPREG , GPREG",{0x0000002A,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01010100  slt $zero, $zero, $zero
{             "slti GPREG , GPREG , NUM",{0x28000000,0x03FFFFFF}}, // xx010100xxxxxxxxxxxxxxxxxxxxxxxx  slti $zero, $zero, 0
{            "sltiu GPREG , GPREG , NUM",{0x2C000000,0x03FFFFFF}}, // xx110100xxxxxxxxxxxxxxxxxxxxxxxx  sltiu $zero, $zero, 0
{           "sltu GPREG , GPREG , GPREG",{0x0000002B,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11010100  sltu $zero, $zero, $zero
{        "splat.b WREG , WREG [ GPREG ]",{0x78800014,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx001010xx  splat.b $w0, $w0[$zero]
{        "splat.d WREG , WREG [ GPREG ]",{0x78E00014,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx001010xx  splat.d $w0, $w0[$zero]
{        "splat.h WREG , WREG [ GPREG ]",{0x78A00014,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx001010xx  splat.h $w0, $w0[$zero]
{        "splat.w WREG , WREG [ GPREG ]",{0x78C00014,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx001010xx  splat.w $w0, $w0[$zero]
{         "splati.b WREG , WREG [ NUM ]",{0x78400019,0x000FFFC0}}, // 00011110xxxx0010xxxxxxxx100110xx  splati.b $w0, $w0[0]
{         "splati.d WREG , WREG [ NUM ]",{0x78780019,0x0001FFC0}}, // 00011110x0011110xxxxxxxx100110xx  splati.d $w0, $w0[0]
{         "splati.h WREG , WREG [ NUM ]",{0x78600019,0x0007FFC0}}, // 00011110xxx00110xxxxxxxx100110xx  splati.h $w0, $w0[0]
{         "splati.w WREG , WREG [ NUM ]",{0x78700019,0x0003FFC0}}, // 00011110xx001110xxxxxxxx100110xx  splati.w $w0, $w0[0]
{                   "sqrt.d FREG , FREG",{0x46200004,0x0000F780}}, // 0110001000000100xxx0xxxx0010000x  sqrt.d $f0, $f0
{                   "sqrt.s FREG , FREG",{0x46000004,0x0000FFC0}}, // 0110001000000000xxxxxxxx001000xx  sqrt.s $f0, $f0
{              "sra GPREG , GPREG , NUM",{0x00000003,0x001FFFC0}}, // 00000000xxxxx000xxxxxxxx110000xx  sra $zero, $zero, 0
{             "sra.b WREG , WREG , WREG",{0x7880000D,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx101100xx  sra.b $w0, $w0, $w0
{             "sra.d WREG , WREG , WREG",{0x78E0000D,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx101100xx  sra.d $w0, $w0, $w0
{             "sra.h WREG , WREG , WREG",{0x78A0000D,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx101100xx  sra.h $w0, $w0, $w0
{             "sra.w WREG , WREG , WREG",{0x78C0000D,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx101100xx  sra.w $w0, $w0, $w0
{             "srai.b WREG , WREG , NUM",{0x78F00009,0x0007FFC0}}, // 00011110xxx01111xxxxxxxx100100xx  srai.b $w0, $w0, 0
{             "srai.d WREG , WREG , NUM",{0x78800009,0x003FFFC0}}, // 00011110xxxxxx01xxxxxxxx100100xx  srai.d $w0, $w0, 0
{             "srai.h WREG , WREG , NUM",{0x78E00009,0x000FFFC0}}, // 00011110xxxx0111xxxxxxxx100100xx  srai.h $w0, $w0, 0
{             "srai.w WREG , WREG , NUM",{0x78C00009,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx100100xx  srai.w $w0, $w0, 0
{            "srar.b WREG , WREG , WREG",{0x78800015,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx101010xx  srar.b $w0, $w0, $w0
{            "srar.d WREG , WREG , WREG",{0x78E00015,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx101010xx  srar.d $w0, $w0, $w0
{            "srar.h WREG , WREG , WREG",{0x78A00015,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx101010xx  srar.h $w0, $w0, $w0
{            "srar.w WREG , WREG , WREG",{0x78C00015,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx101010xx  srar.w $w0, $w0, $w0
{            "srari.b WREG , WREG , NUM",{0x7970000A,0x0007FFC0}}, // 10011110xxx01110xxxxxxxx010100xx  srari.b $w0, $w0, 0
{            "srari.d WREG , WREG , NUM",{0x7900000A,0x003FFFC0}}, // 10011110xxxxxx00xxxxxxxx010100xx  srari.d $w0, $w0, 0
{            "srari.h WREG , WREG , NUM",{0x7960000A,0x000FFFC0}}, // 10011110xxxx0110xxxxxxxx010100xx  srari.h $w0, $w0, 0
{            "srari.w WREG , WREG , NUM",{0x7940000A,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx010100xx  srari.w $w0, $w0, 0
{           "srav GPREG , GPREG , GPREG",{0x00000007,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11100000  srav $zero, $zero, $zero
{              "srl GPREG , GPREG , NUM",{0x00000002,0x001FFFC0}}, // 00000000xxxxx000xxxxxxxx010000xx  srl $zero, $zero, 0
{             "srl.b WREG , WREG , WREG",{0x7900000D,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx101100xx  srl.b $w0, $w0, $w0
{             "srl.d WREG , WREG , WREG",{0x7960000D,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx101100xx  srl.d $w0, $w0, $w0
{             "srl.h WREG , WREG , WREG",{0x7920000D,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx101100xx  srl.h $w0, $w0, $w0
{             "srl.w WREG , WREG , WREG",{0x7940000D,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx101100xx  srl.w $w0, $w0, $w0
{             "srli.b WREG , WREG , NUM",{0x79700009,0x0007FFC0}}, // 10011110xxx01110xxxxxxxx100100xx  srli.b $w0, $w0, 0
{             "srli.d WREG , WREG , NUM",{0x79000009,0x003FFFC0}}, // 10011110xxxxxx00xxxxxxxx100100xx  srli.d $w0, $w0, 0
{             "srli.h WREG , WREG , NUM",{0x79600009,0x000FFFC0}}, // 10011110xxxx0110xxxxxxxx100100xx  srli.h $w0, $w0, 0
{             "srli.w WREG , WREG , NUM",{0x79400009,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx100100xx  srli.w $w0, $w0, 0
{            "srlr.b WREG , WREG , WREG",{0x79000015,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx101010xx  srlr.b $w0, $w0, $w0
{            "srlr.d WREG , WREG , WREG",{0x79600015,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx101010xx  srlr.d $w0, $w0, $w0
{            "srlr.h WREG , WREG , WREG",{0x79200015,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx101010xx  srlr.h $w0, $w0, $w0
{            "srlr.w WREG , WREG , WREG",{0x79400015,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx101010xx  srlr.w $w0, $w0, $w0
{            "srlri.b WREG , WREG , NUM",{0x79F0000A,0x0007FFC0}}, // 10011110xxx01111xxxxxxxx010100xx  srlri.b $w0, $w0, 0
{            "srlri.d WREG , WREG , NUM",{0x7980000A,0x003FFFC0}}, // 10011110xxxxxx01xxxxxxxx010100xx  srlri.d $w0, $w0, 0
{            "srlri.h WREG , WREG , NUM",{0x79E0000A,0x000FFFC0}}, // 10011110xxxx0111xxxxxxxx010100xx  srlri.h $w0, $w0, 0
{            "srlri.w WREG , WREG , NUM",{0x79C0000A,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx010100xx  srlri.w $w0, $w0, 0
{           "srlv GPREG , GPREG , GPREG",{0x00000006,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01100000  srlv $zero, $zero, $zero
{                                "ssnop",{0x00000040,0x00000000}}, // 00000000000000000000000000000010  ssnop
{                "st.b WREG , ( GPREG )",{0x78000024,0x0000FFC0}}, // 0001111000000000xxxxxxxx001001xx  st.b $w0, ($zero)
{            "st.b WREG , NUM ( GPREG )",{0x7A000024,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx001001xx  st.b $w0, -0x200($zero)
{                "st.d WREG , ( GPREG )",{0x78000027,0x0000FFC0}}, // 0001111000000000xxxxxxxx111001xx  st.d $w0, ($zero)
{            "st.d WREG , NUM ( GPREG )",{0x7A000027,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx111001xx  st.d $w0, -0x1000($zero)
{                "st.h WREG , ( GPREG )",{0x78000025,0x0000FFC0}}, // 0001111000000000xxxxxxxx101001xx  st.h $w0, ($zero)
{            "st.h WREG , NUM ( GPREG )",{0x7A000025,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx101001xx  st.h $w0, -0x400($zero)
{                "st.w WREG , ( GPREG )",{0x78000026,0x0000FFC0}}, // 0001111000000000xxxxxxxx011001xx  st.w $w0, ($zero)
{            "st.w WREG , NUM ( GPREG )",{0x7A000026,0x03FFFFC0}}, // xx011110xxxxxxxxxxxxxxxx011001xx  st.w $w0, -0x800($zero)
{            "sub GPREG , GPREG , GPREG",{0x01000022,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01000100  sub $zero, $t0, $zero
{             "sub.d FREG , FREG , FREG",{0x46200001,0x001EF780}}, // 011000100xxxx100xxx0xxxx1000000x  sub.d $f0, $f0, $f0
{             "sub.s FREG , FREG , FREG",{0x46000001,0x001FFFC0}}, // 01100010xxxxx000xxxxxxxx100000xx  sub.s $f0, $f0, $f0
{        "subq.ph GPREG , GPREG , GPREG",{0x7C0002D0,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00001011  subq.ph $zero, $zero, $zero
{      "subq_s.ph GPREG , GPREG , GPREG",{0x7C0003D0,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00001011  subq_s.ph $zero, $zero, $zero
{       "subq_s.w GPREG , GPREG , GPREG",{0x7C0005D0,0x03FFF800}}, // xx111110xxxxxxxx101xxxxx00001011  subq_s.w $zero, $zero, $zero
{       "subqh.ph GPREG , GPREG , GPREG",{0x7C000258,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00011010  subqh.ph $zero, $zero, $zero
{        "subqh.w GPREG , GPREG , GPREG",{0x7C000458,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011010  subqh.w $zero, $zero, $zero
{     "subqh_r.ph GPREG , GPREG , GPREG",{0x7C0002D8,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00011011  subqh_r.ph $zero, $zero, $zero
{      "subqh_r.w GPREG , GPREG , GPREG",{0x7C0004D8,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011011  subqh_r.w $zero, $zero, $zero
{          "subs_s.b WREG , WREG , WREG",{0x78000011,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx100010xx  subs_s.b $w0, $w0, $w0
{          "subs_s.d WREG , WREG , WREG",{0x78600011,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx100010xx  subs_s.d $w0, $w0, $w0
{          "subs_s.h WREG , WREG , WREG",{0x78200011,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx100010xx  subs_s.h $w0, $w0, $w0
{          "subs_s.w WREG , WREG , WREG",{0x78400011,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx100010xx  subs_s.w $w0, $w0, $w0
{          "subs_u.b WREG , WREG , WREG",{0x78800011,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx100010xx  subs_u.b $w0, $w0, $w0
{          "subs_u.d WREG , WREG , WREG",{0x78E00011,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx100010xx  subs_u.d $w0, $w0, $w0
{          "subs_u.h WREG , WREG , WREG",{0x78A00011,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx100010xx  subs_u.h $w0, $w0, $w0
{          "subs_u.w WREG , WREG , WREG",{0x78C00011,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx100010xx  subs_u.w $w0, $w0, $w0
{        "subsus_u.b WREG , WREG , WREG",{0x79000011,0x001FFFC0}}, // 10011110xxxxx000xxxxxxxx100010xx  subsus_u.b $w0, $w0, $w0
{        "subsus_u.d WREG , WREG , WREG",{0x79600011,0x001FFFC0}}, // 10011110xxxxx110xxxxxxxx100010xx  subsus_u.d $w0, $w0, $w0
{        "subsus_u.h WREG , WREG , WREG",{0x79200011,0x001FFFC0}}, // 10011110xxxxx100xxxxxxxx100010xx  subsus_u.h $w0, $w0, $w0
{        "subsus_u.w WREG , WREG , WREG",{0x79400011,0x001FFFC0}}, // 10011110xxxxx010xxxxxxxx100010xx  subsus_u.w $w0, $w0, $w0
{        "subsuu_s.b WREG , WREG , WREG",{0x79800011,0x001FFFC0}}, // 10011110xxxxx001xxxxxxxx100010xx  subsuu_s.b $w0, $w0, $w0
{        "subsuu_s.d WREG , WREG , WREG",{0x79E00011,0x001FFFC0}}, // 10011110xxxxx111xxxxxxxx100010xx  subsuu_s.d $w0, $w0, $w0
{        "subsuu_s.h WREG , WREG , WREG",{0x79A00011,0x001FFFC0}}, // 10011110xxxxx101xxxxxxxx100010xx  subsuu_s.h $w0, $w0, $w0
{        "subsuu_s.w WREG , WREG , WREG",{0x79C00011,0x001FFFC0}}, // 10011110xxxxx011xxxxxxxx100010xx  subsuu_s.w $w0, $w0, $w0
{           "subu GPREG , GPREG , GPREG",{0x01000023,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11000100  subu $zero, $t0, $zero
{        "subu.ph GPREG , GPREG , GPREG",{0x7C000250,0x03FFF800}}, // xx111110xxxxxxxx010xxxxx00001010  subu.ph $zero, $zero, $zero
{        "subu.qb GPREG , GPREG , GPREG",{0x7C000050,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00001010  subu.qb $zero, $zero, $zero
{      "subu_s.ph GPREG , GPREG , GPREG",{0x7C000350,0x03FFF800}}, // xx111110xxxxxxxx110xxxxx00001010  subu_s.ph $zero, $zero, $zero
{      "subu_s.qb GPREG , GPREG , GPREG",{0x7C000150,0x03FFF800}}, // xx111110xxxxxxxx100xxxxx00001010  subu_s.qb $zero, $zero, $zero
{       "subuh.qb GPREG , GPREG , GPREG",{0x7C000058,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00011010  subuh.qb $zero, $zero, $zero
{     "subuh_r.qb GPREG , GPREG , GPREG",{0x7C0000D8,0x03FFF800}}, // xx111110xxxxxxxx000xxxxx00011011  subuh_r.qb $zero, $zero, $zero
{            "subv.b WREG , WREG , WREG",{0x7880000E,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx011100xx  subv.b $w0, $w0, $w0
{            "subv.d WREG , WREG , WREG",{0x78E0000E,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx011100xx  subv.d $w0, $w0, $w0
{            "subv.h WREG , WREG , WREG",{0x78A0000E,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx011100xx  subv.h $w0, $w0, $w0
{            "subv.w WREG , WREG , WREG",{0x78C0000E,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx011100xx  subv.w $w0, $w0, $w0
{            "subvi.b WREG , WREG , NUM",{0x78800006,0x001FFFC0}}, // 00011110xxxxx001xxxxxxxx011000xx  subvi.b $w0, $w0, 0
{            "subvi.d WREG , WREG , NUM",{0x78E00006,0x001FFFC0}}, // 00011110xxxxx111xxxxxxxx011000xx  subvi.d $w0, $w0, 0
{            "subvi.h WREG , WREG , NUM",{0x78A00006,0x001FFFC0}}, // 00011110xxxxx101xxxxxxxx011000xx  subvi.h $w0, $w0, 0
{            "subvi.w WREG , WREG , NUM",{0x78C00006,0x001FFFC0}}, // 00011110xxxxx011xxxxxxxx011000xx  subvi.w $w0, $w0, 0
{                 "sw GPREG , ( GPREG )",{0xAC000000,0x03FF0000}}, // xx110101xxxxxxxx0000000000000000  sw $zero, ($zero)
{             "sw GPREG , NUM ( GPREG )",{0xAC000100,0x03FFFFFF}}, // xx110101xxxxxxxxxxxxxxxxxxxxxxxx  sw $zero, 0x100($zero)
{                "swc1 FREG , ( GPREG )",{0xE4000000,0x03FF0000}}, // xx100111xxxxxxxx0000000000000000  swc1 $f0, ($zero)
{            "swc1 FREG , NUM ( GPREG )",{0xE4000100,0x03FFFFFF}}, // xx100111xxxxxxxxxxxxxxxxxxxxxxxx  swc1 $f0, 0x100($zero)
{                      "swc2 CASH , ( )",{0x49600000,0x001F0000}}, // 10010010xxxxx1100000000000000000  swc2 $0, ()
{                  "swc2 CASH , ( NUM )",{0x49600100,0x001FFFFF}}, // 10010010xxxxx110xxxxxxxxxxxxxxxx  swc2 $0, (0x100)
{                                 "sync",{0x0000000F,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx11110000  sync
{                             "sync NUM",{0x0000010F,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx111100xx  sync 4
{                              "syscall",{0x0000000C,0x00000000}}, // 00000000000000000000000000110000  syscall
{                          "syscall NUM",{0x0200000C,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx001100xx  syscall 0x80000
{                    "teq GPREG , GPREG",{0x00000034,0x03FF0000}}, // xx000000xxxxxxxx0000000000101100  teq $zero, $zero
{              "teq GPREG , GPREG , NUM",{0x00000134,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx001011xx  teq $zero, $zero, 4
{                    "tge GPREG , GPREG",{0x00000030,0x03FF0000}}, // xx000000xxxxxxxx0000000000001100  tge $zero, $zero
{              "tge GPREG , GPREG , NUM",{0x00000130,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx000011xx  tge $zero, $zero, 4
{                   "tgeu GPREG , GPREG",{0x00000031,0x03FF0000}}, // xx000000xxxxxxxx0000000010001100  tgeu $zero, $zero
{             "tgeu GPREG , GPREG , NUM",{0x00000131,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx100011xx  tgeu $zero, $zero, 4
{                                 "tlbp",{0x42000008,0x00000000}}, // 01000010000000000000000000010000  tlbp
{                                 "tlbr",{0x42000001,0x00000000}}, // 01000010000000000000000010000000  tlbr
{                                "tlbwi",{0x42000002,0x00000000}}, // 01000010000000000000000001000000  tlbwi
{                                "tlbwr",{0x42000006,0x00000000}}, // 01000010000000000000000001100000  tlbwr
{                    "tlt GPREG , GPREG",{0x00000032,0x03FF0000}}, // xx000000xxxxxxxx0000000001001100  tlt $zero, $zero
{              "tlt GPREG , GPREG , NUM",{0x00000132,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx010011xx  tlt $zero, $zero, 4
{                   "tltu GPREG , GPREG",{0x00000033,0x03FF0000}}, // xx000000xxxxxxxx0000000011001100  tltu $zero, $zero
{             "tltu GPREG , GPREG , NUM",{0x00000133,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx110011xx  tltu $zero, $zero, 4
{                    "tne GPREG , GPREG",{0x00000036,0x03FF0000}}, // xx000000xxxxxxxx0000000001101100  tne $zero, $zero
{              "tne GPREG , GPREG , NUM",{0x00000136,0x03FFFFC0}}, // xx000000xxxxxxxxxxxxxxxx011011xx  tne $zero, $zero, 4
{                "trunc.w.d FREG , FREG",{0x4620000D,0x0000F7C0}}, // 0110001000000100xxx0xxxx101100xx  trunc.w.d $f0, $f0
{                "trunc.w.s FREG , FREG",{0x4600000D,0x0000FFC0}}, // 0110001000000000xxxxxxxx101100xx  trunc.w.s $f0, $f0
{                                "undef",{0x01000000,0xFFFFFFFF}}, // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  undef
{            "vshf.b WREG , WREG , WREG",{0x78000015,0x001FFFC0}}, // 00011110xxxxx000xxxxxxxx101010xx  vshf.b $w0, $w0, $w0
{            "vshf.d WREG , WREG , WREG",{0x78600015,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx101010xx  vshf.d $w0, $w0, $w0
{            "vshf.h WREG , WREG , WREG",{0x78200015,0x001FFFC0}}, // 00011110xxxxx100xxxxxxxx101010xx  vshf.h $w0, $w0, $w0
{            "vshf.w WREG , WREG , WREG",{0x78400015,0x001FFFC0}}, // 00011110xxxxx010xxxxxxxx101010xx  vshf.w $w0, $w0, $w0
{                                 "wait",{0x42000020,0x00000000}}, // 01000010000000000000000000000100  wait
{                    "wrdsp GPREG , NUM",{0x7C0004F8,0x03FFF800}}, // xx111110xxxxxxxx001xxxxx00011111  wrdsp $zero, 0
{                   "wsbh GPREG , GPREG",{0x7C0000A0,0x001FF800}}, // 00111110xxxxx000000xxxxx00000101  wsbh $zero, $zero
{            "xor GPREG , GPREG , GPREG",{0x00000026,0x03FFF800}}, // xx000000xxxxxxxx000xxxxx01100100  xor $zero, $zero, $zero
{             "xor.v WREG , WREG , WREG",{0x7860001E,0x001FFFC0}}, // 00011110xxxxx110xxxxxxxx011110xx  xor.v $w0, $w0, $w0
{             "xori GPREG , GPREG , NUM",{0x38000000,0x03FFFFFF}}, // xx011100xxxxxxxxxxxxxxxxxxxxxxxx  xori $zero, $zero, 0
{             "xori.b WREG , WREG , NUM",{0x7B000000,0x00FFFFC0}}, // 11011110xxxxxxxxxxxxxxxx000000xx  xori.b $w0, $w0, 0
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
	while(inbuf[n]=='.' || isalnum(inbuf[n]))
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
				result.push_back({TT_FREG, value, ""});
				inbuf = endptr;
			}
			/* acreg's */
			else if(d=='a' && e=='c') {
				uint32_t value = strtoul(inbuf+3, &endptr, 10);
				result.push_back({TT_FREG, value, ""});
				inbuf = endptr;
			}
			/* $zero */
			else if(d=='z' && e=='e') {
				if(strncmp(inbuf, "$zero", 5)) {
					err = "expected $zero";
					MYLOG("ERROR: %s", err.c_str());
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
					MYLOG("ERROR: %s", err.c_str());
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
			MYLOG("ERROR: %s", err.c_str());
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
	return insword;
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
	if(toks_src[0].sval[0]=='b' && toks_src[0].sval.back() != 'a' && toks_src.back().type == TT_NUM) {
		toks_src.back().ival -= addr;
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

		for(; b1i<n_flips; b1i = (b1i+1) % n_flips) {
			uint32_t child = parent ^ flipper[b1i];
			child = special_handling(info.seed, child, flipper_idx[b1i]);

			//MYLOG("b1i is now: %d\n", b1i);

			float s = score(toks_src, child, addr);
			if(s > top_score) {
				parent = child;
				top_score = s;
				overtake = true;
				break;
			}
	
			failures++;
			if(failures > FAILURES_LIMIT) {
				MYLOG("failure limit reached, not assembling!\n");
				err = "cannot assemble, valid operands?";
				goto cleanup;
			}

			failstreak++;
			MYLOG("--failstreak is now: %d\n", failstreak);
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
			if(1) {
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
			printf("ERROR: %s", err.c_str());
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
				printf("ERROR: %s", err.c_str());
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
