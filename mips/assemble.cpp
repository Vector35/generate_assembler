
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
{                    "abs.d FREG , FREG",{0x05002046,0x80F70000}}, // x0000101xxxx0xxx0010000001000110  abs.d $f0, $f0
{                    "abs.s FREG , FREG",{0x05000046,0xC0FF0000}}, // xx000101xxxxxxxx0000000001000110  abs.s $f0, $f0
{              "absq_s.ph GPREG , GPREG",{0x5202007C,0x00F81F00}}, // 01010010xxxxx010000xxxxx01111100  absq_s.ph $zero, $zero
{              "absq_s.qb GPREG , GPREG",{0x5200007C,0x00F81F00}}, // 01010010xxxxx000000xxxxx01111100  absq_s.qb $zero, $zero
{               "absq_s.w GPREG , GPREG",{0x5204007C,0x00F81F00}}, // 01010010xxxxx100000xxxxx01111100  absq_s.w $zero, $zero
{            "add GPREG , GPREG , GPREG",{0x20000000,0x00F8FF03}}, // 00100000xxxxx000xxxxxxxx000000xx  add $zero, $zero, $zero
{             "add.d FREG , FREG , FREG",{0x00002046,0x80F71E00}}, // x0000000xxxx0xxx001xxxx001000110  add.d $f0, $f0, $f0
{             "add.s FREG , FREG , FREG",{0x00000046,0xC0FF1F00}}, // xx000000xxxxxxxx000xxxxx01000110  add.s $f0, $f0, $f0
{           "add_a.b WREG , WREG , WREG",{0x10000078,0xC0FF1F00}}, // xx010000xxxxxxxx000xxxxx01111000  add_a.b $w0, $w0, $w0
{           "add_a.d WREG , WREG , WREG",{0x10006078,0xC0FF1F00}}, // xx010000xxxxxxxx011xxxxx01111000  add_a.d $w0, $w0, $w0
{           "add_a.h WREG , WREG , WREG",{0x10002078,0xC0FF1F00}}, // xx010000xxxxxxxx001xxxxx01111000  add_a.h $w0, $w0, $w0
{           "add_a.w WREG , WREG , WREG",{0x10004078,0xC0FF1F00}}, // xx010000xxxxxxxx010xxxxx01111000  add_a.w $w0, $w0, $w0
{            "addiu GPREG , GPREG , NUM",{0x00000024,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001001xx  addiu $zero, $zero, 0
{                  "addiupc GPREG , NUM",{0x000000EC,0xFFFFE703}}, // xxxxxxxxxxxxxxxxxxx00xxx111011xx  addiupc $zero, 0
{        "addq.ph GPREG , GPREG , GPREG",{0x9002007C,0x00F8FF03}}, // 10010000xxxxx010xxxxxxxx011111xx  addq.ph $zero, $zero, $zero
{      "addq_s.ph GPREG , GPREG , GPREG",{0x9003007C,0x00F8FF03}}, // 10010000xxxxx011xxxxxxxx011111xx  addq_s.ph $zero, $zero, $zero
{       "addq_s.w GPREG , GPREG , GPREG",{0x9005007C,0x00F8FF03}}, // 10010000xxxxx101xxxxxxxx011111xx  addq_s.w $zero, $zero, $zero
{       "addqh.ph GPREG , GPREG , GPREG",{0x1802007C,0x00F8FF03}}, // 00011000xxxxx010xxxxxxxx011111xx  addqh.ph $zero, $zero, $zero
{        "addqh.w GPREG , GPREG , GPREG",{0x1804007C,0x00F8FF03}}, // 00011000xxxxx100xxxxxxxx011111xx  addqh.w $zero, $zero, $zero
{     "addqh_r.ph GPREG , GPREG , GPREG",{0x9802007C,0x00F8FF03}}, // 10011000xxxxx010xxxxxxxx011111xx  addqh_r.ph $zero, $zero, $zero
{      "addqh_r.w GPREG , GPREG , GPREG",{0x9804007C,0x00F8FF03}}, // 10011000xxxxx100xxxxxxxx011111xx  addqh_r.w $zero, $zero, $zero
{          "adds_a.b WREG , WREG , WREG",{0x10008078,0xC0FF1F00}}, // xx010000xxxxxxxx100xxxxx01111000  adds_a.b $w0, $w0, $w0
{          "adds_a.d WREG , WREG , WREG",{0x1000E078,0xC0FF1F00}}, // xx010000xxxxxxxx111xxxxx01111000  adds_a.d $w0, $w0, $w0
{          "adds_a.h WREG , WREG , WREG",{0x1000A078,0xC0FF1F00}}, // xx010000xxxxxxxx101xxxxx01111000  adds_a.h $w0, $w0, $w0
{          "adds_a.w WREG , WREG , WREG",{0x1000C078,0xC0FF1F00}}, // xx010000xxxxxxxx110xxxxx01111000  adds_a.w $w0, $w0, $w0
{          "adds_s.b WREG , WREG , WREG",{0x10000079,0xC0FF1F00}}, // xx010000xxxxxxxx000xxxxx01111001  adds_s.b $w0, $w0, $w0
{          "adds_s.d WREG , WREG , WREG",{0x10006079,0xC0FF1F00}}, // xx010000xxxxxxxx011xxxxx01111001  adds_s.d $w0, $w0, $w0
{          "adds_s.h WREG , WREG , WREG",{0x10002079,0xC0FF1F00}}, // xx010000xxxxxxxx001xxxxx01111001  adds_s.h $w0, $w0, $w0
{          "adds_s.w WREG , WREG , WREG",{0x10004079,0xC0FF1F00}}, // xx010000xxxxxxxx010xxxxx01111001  adds_s.w $w0, $w0, $w0
{          "adds_u.b WREG , WREG , WREG",{0x10008079,0xC0FF1F00}}, // xx010000xxxxxxxx100xxxxx01111001  adds_u.b $w0, $w0, $w0
{          "adds_u.d WREG , WREG , WREG",{0x1000E079,0xC0FF1F00}}, // xx010000xxxxxxxx111xxxxx01111001  adds_u.d $w0, $w0, $w0
{          "adds_u.h WREG , WREG , WREG",{0x1000A079,0xC0FF1F00}}, // xx010000xxxxxxxx101xxxxx01111001  adds_u.h $w0, $w0, $w0
{          "adds_u.w WREG , WREG , WREG",{0x1000C079,0xC0FF1F00}}, // xx010000xxxxxxxx110xxxxx01111001  adds_u.w $w0, $w0, $w0
{          "addsc GPREG , GPREG , GPREG",{0x1004007C,0x00F8FF03}}, // 00010000xxxxx100xxxxxxxx011111xx  addsc $zero, $zero, $zero
{           "addu GPREG , GPREG , GPREG",{0x21000100,0x00F8FF03}}, // 00100001xxxxx000xxxxxxxx000000xx  addu $zero, $zero, $at
{        "addu.ph GPREG , GPREG , GPREG",{0x1002007C,0x00F8FF03}}, // 00010000xxxxx010xxxxxxxx011111xx  addu.ph $zero, $zero, $zero
{        "addu.qb GPREG , GPREG , GPREG",{0x1000007C,0x00F8FF03}}, // 00010000xxxxx000xxxxxxxx011111xx  addu.qb $zero, $zero, $zero
{      "addu_s.ph GPREG , GPREG , GPREG",{0x1003007C,0x00F8FF03}}, // 00010000xxxxx011xxxxxxxx011111xx  addu_s.ph $zero, $zero, $zero
{      "addu_s.qb GPREG , GPREG , GPREG",{0x1001007C,0x00F8FF03}}, // 00010000xxxxx001xxxxxxxx011111xx  addu_s.qb $zero, $zero, $zero
{       "adduh.qb GPREG , GPREG , GPREG",{0x1800007C,0x00F8FF03}}, // 00011000xxxxx000xxxxxxxx011111xx  adduh.qb $zero, $zero, $zero
{     "adduh_r.qb GPREG , GPREG , GPREG",{0x9800007C,0x00F8FF03}}, // 10011000xxxxx000xxxxxxxx011111xx  adduh_r.qb $zero, $zero, $zero
{            "addv.b WREG , WREG , WREG",{0x0E000078,0xC0FF1F00}}, // xx001110xxxxxxxx000xxxxx01111000  addv.b $w0, $w0, $w0
{            "addv.d WREG , WREG , WREG",{0x0E006078,0xC0FF1F00}}, // xx001110xxxxxxxx011xxxxx01111000  addv.d $w0, $w0, $w0
{            "addv.h WREG , WREG , WREG",{0x0E002078,0xC0FF1F00}}, // xx001110xxxxxxxx001xxxxx01111000  addv.h $w0, $w0, $w0
{            "addv.w WREG , WREG , WREG",{0x0E004078,0xC0FF1F00}}, // xx001110xxxxxxxx010xxxxx01111000  addv.w $w0, $w0, $w0
{            "addvi.b WREG , WREG , NUM",{0x06000078,0xC0FF1F00}}, // xx000110xxxxxxxx000xxxxx01111000  addvi.b $w0, $w0, 0
{            "addvi.d WREG , WREG , NUM",{0x06006078,0xC0FF1F00}}, // xx000110xxxxxxxx011xxxxx01111000  addvi.d $w0, $w0, 0
{            "addvi.h WREG , WREG , NUM",{0x06002078,0xC0FF1F00}}, // xx000110xxxxxxxx001xxxxx01111000  addvi.h $w0, $w0, 0
{            "addvi.w WREG , WREG , NUM",{0x06004078,0xC0FF1F00}}, // xx000110xxxxxxxx010xxxxx01111000  addvi.w $w0, $w0, 0
{          "addwc GPREG , GPREG , GPREG",{0x5004007C,0x00F8FF03}}, // 01010000xxxxx100xxxxxxxx011111xx  addwc $zero, $zero, $zero
{    "align GPREG , GPREG , GPREG , NUM",{0x2002007C,0xC0F8FF03}}, // xx100000xxxxx010xxxxxxxx011111xx  align $zero, $zero, $zero, 0
{                   "aluipc GPREG , NUM",{0x00001FEC,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx11111111011xx  aluipc $zero, 0
{            "and GPREG , GPREG , GPREG",{0x24000000,0x00F8FF03}}, // 00100100xxxxx000xxxxxxxx000000xx  and $zero, $zero, $zero
{             "and.v WREG , WREG , WREG",{0x1E000078,0xC0FF1F00}}, // xx011110xxxxxxxx000xxxxx01111000  and.v $w0, $w0, $w0
{             "andi GPREG , GPREG , NUM",{0x00000030,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001100xx  andi $zero, $zero, 0
{             "andi.b WREG , WREG , NUM",{0x00000078,0xC0FFFF00}}, // xx000000xxxxxxxxxxxxxxxx01111000  andi.b $w0, $w0, 0
{           "append GPREG , GPREG , NUM",{0x3100007C,0x00F8FF03}}, // 00110001xxxxx000xxxxxxxx011111xx  append $zero, $zero, 0
{          "asub_s.b WREG , WREG , WREG",{0x1100007A,0xC0FF1F00}}, // xx010001xxxxxxxx000xxxxx01111010  asub_s.b $w0, $w0, $w0
{          "asub_s.d WREG , WREG , WREG",{0x1100607A,0xC0FF1F00}}, // xx010001xxxxxxxx011xxxxx01111010  asub_s.d $w0, $w0, $w0
{          "asub_s.h WREG , WREG , WREG",{0x1100207A,0xC0FF1F00}}, // xx010001xxxxxxxx001xxxxx01111010  asub_s.h $w0, $w0, $w0
{          "asub_s.w WREG , WREG , WREG",{0x1100407A,0xC0FF1F00}}, // xx010001xxxxxxxx010xxxxx01111010  asub_s.w $w0, $w0, $w0
{          "asub_u.b WREG , WREG , WREG",{0x1100807A,0xC0FF1F00}}, // xx010001xxxxxxxx100xxxxx01111010  asub_u.b $w0, $w0, $w0
{          "asub_u.d WREG , WREG , WREG",{0x1100E07A,0xC0FF1F00}}, // xx010001xxxxxxxx111xxxxx01111010  asub_u.d $w0, $w0, $w0
{          "asub_u.h WREG , WREG , WREG",{0x1100A07A,0xC0FF1F00}}, // xx010001xxxxxxxx101xxxxx01111010  asub_u.h $w0, $w0, $w0
{          "asub_u.w WREG , WREG , WREG",{0x1100C07A,0xC0FF1F00}}, // xx010001xxxxxxxx110xxxxx01111010  asub_u.w $w0, $w0, $w0
{              "aui GPREG , GPREG , NUM",{0x0000003C,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001111xx  aui $zero, $zero, 0
{                    "auipc GPREG , NUM",{0x00001EEC,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx11110111011xx  auipc $zero, 0
{           "ave_s.b WREG , WREG , WREG",{0x1000007A,0xC0FF1F00}}, // xx010000xxxxxxxx000xxxxx01111010  ave_s.b $w0, $w0, $w0
{           "ave_s.d WREG , WREG , WREG",{0x1000607A,0xC0FF1F00}}, // xx010000xxxxxxxx011xxxxx01111010  ave_s.d $w0, $w0, $w0
{           "ave_s.h WREG , WREG , WREG",{0x1000207A,0xC0FF1F00}}, // xx010000xxxxxxxx001xxxxx01111010  ave_s.h $w0, $w0, $w0
{           "ave_s.w WREG , WREG , WREG",{0x1000407A,0xC0FF1F00}}, // xx010000xxxxxxxx010xxxxx01111010  ave_s.w $w0, $w0, $w0
{           "ave_u.b WREG , WREG , WREG",{0x1000807A,0xC0FF1F00}}, // xx010000xxxxxxxx100xxxxx01111010  ave_u.b $w0, $w0, $w0
{           "ave_u.d WREG , WREG , WREG",{0x1000E07A,0xC0FF1F00}}, // xx010000xxxxxxxx111xxxxx01111010  ave_u.d $w0, $w0, $w0
{           "ave_u.h WREG , WREG , WREG",{0x1000A07A,0xC0FF1F00}}, // xx010000xxxxxxxx101xxxxx01111010  ave_u.h $w0, $w0, $w0
{           "ave_u.w WREG , WREG , WREG",{0x1000C07A,0xC0FF1F00}}, // xx010000xxxxxxxx110xxxxx01111010  ave_u.w $w0, $w0, $w0
{          "aver_s.b WREG , WREG , WREG",{0x1000007B,0xC0FF1F00}}, // xx010000xxxxxxxx000xxxxx01111011  aver_s.b $w0, $w0, $w0
{          "aver_s.d WREG , WREG , WREG",{0x1000607B,0xC0FF1F00}}, // xx010000xxxxxxxx011xxxxx01111011  aver_s.d $w0, $w0, $w0
{          "aver_s.h WREG , WREG , WREG",{0x1000207B,0xC0FF1F00}}, // xx010000xxxxxxxx001xxxxx01111011  aver_s.h $w0, $w0, $w0
{          "aver_s.w WREG , WREG , WREG",{0x1000407B,0xC0FF1F00}}, // xx010000xxxxxxxx010xxxxx01111011  aver_s.w $w0, $w0, $w0
{          "aver_u.b WREG , WREG , WREG",{0x1000807B,0xC0FF1F00}}, // xx010000xxxxxxxx100xxxxx01111011  aver_u.b $w0, $w0, $w0
{          "aver_u.d WREG , WREG , WREG",{0x1000E07B,0xC0FF1F00}}, // xx010000xxxxxxxx111xxxxx01111011  aver_u.d $w0, $w0, $w0
{          "aver_u.h WREG , WREG , WREG",{0x1000A07B,0xC0FF1F00}}, // xx010000xxxxxxxx101xxxxx01111011  aver_u.h $w0, $w0, $w0
{          "aver_u.w WREG , WREG , WREG",{0x1000C07B,0xC0FF1F00}}, // xx010000xxxxxxxx110xxxxx01111011  aver_u.w $w0, $w0, $w0
{                                "b NUM",{0x00000010,0xFFFF0000}}, // xxxxxxxxxxxxxxxx0000000000010000  b 4
{                              "bal NUM",{0x00001104,0xFFFF0000}}, // xxxxxxxxxxxxxxxx0001000100000100  bal 4
{                             "balc NUM",{0x000000E8,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx111010xx  balc 0
{           "balign GPREG , GPREG , NUM",{0x3104007C,0x00F8FF03}}, // 00110001xxxxx100xxxxxxxx011111xx  balign $zero, $zero, 0
{                               "bc NUM",{0x000000C8,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx110010xx  bc 0
{                    "bc1eqz FREG , NUM",{0x00002045,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx001xxxxx01000101  bc1eqz $f0, 4
{                    "bc1nez FREG , NUM",{0x0000A045,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx101xxxxx01000101  bc1nez $f0, 4
{                    "bc2eqz CASH , NUM",{0x00002049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx001xxxxx01001001  bc2eqz $0, 4
{                    "bc2nez CASH , NUM",{0x0000A049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx101xxxxx01001001  bc2nez $0, 4
{            "bclr.b WREG , WREG , WREG",{0x0D008079,0xC0FF1F00}}, // xx001101xxxxxxxx100xxxxx01111001  bclr.b $w0, $w0, $w0
{            "bclr.d WREG , WREG , WREG",{0x0D00E079,0xC0FF1F00}}, // xx001101xxxxxxxx111xxxxx01111001  bclr.d $w0, $w0, $w0
{            "bclr.h WREG , WREG , WREG",{0x0D00A079,0xC0FF1F00}}, // xx001101xxxxxxxx101xxxxx01111001  bclr.h $w0, $w0, $w0
{            "bclr.w WREG , WREG , WREG",{0x0D00C079,0xC0FF1F00}}, // xx001101xxxxxxxx110xxxxx01111001  bclr.w $w0, $w0, $w0
{            "bclri.b WREG , WREG , NUM",{0x0900F079,0xC0FF0700}}, // xx001001xxxxxxxx11110xxx01111001  bclri.b $w0, $w0, 0
{            "bclri.d WREG , WREG , NUM",{0x09008079,0xC0FF3F00}}, // xx001001xxxxxxxx10xxxxxx01111001  bclri.d $w0, $w0, 0
{            "bclri.h WREG , WREG , NUM",{0x0900E079,0xC0FF0F00}}, // xx001001xxxxxxxx1110xxxx01111001  bclri.h $w0, $w0, 0
{            "bclri.w WREG , WREG , NUM",{0x0900C079,0xC0FF1F00}}, // xx001001xxxxxxxx110xxxxx01111001  bclri.w $w0, $w0, 0
{              "beq GPREG , GPREG , NUM",{0x00000110,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000100xx  beq $zero, $at, 4
{             "beqc GPREG , GPREG , NUM",{0x00000921,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001000xx  beqc $t0, $t1, 0
{             "beql GPREG , GPREG , NUM",{0x00000051,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx010100xx  beql $t0, $zero, 4
{                     "beqz GPREG , NUM",{0x00000011,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000000100xx  beqz $t0, 4
{                  "beqzalc GPREG , NUM",{0x00000120,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx00100000  beqzalc $at, 0
{                    "beqzc GPREG , NUM",{0x000000D9,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx110110xx  beqzc $t0, 0
{                    "beqzl GPREG , NUM",{0x00000050,0xFFFF0000}}, // xxxxxxxxxxxxxxxx0000000001010000  beqzl $zero, 4
{             "bgec GPREG , GPREG , NUM",{0x00000159,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx010110xx  bgec $t0, $at, 0
{            "bgeuc GPREG , GPREG , NUM",{0x00000119,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000110xx  bgeuc $t0, $at, 0
{                     "bgez GPREG , NUM",{0x00000104,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00001000001xx  bgez $zero, 4
{                  "bgezalc GPREG , NUM",{0x00000819,0xFFFFF702}}, // xxxxxxxxxxxxxxxxxxxx1xxx000110x1  bgezalc $t0, 0
{                  "bgezall GPREG , NUM",{0x00001304,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx10011000001xx  bgezall $zero, 4
{                    "bgezc GPREG , NUM",{0x00000859,0xFFFFF702}}, // xxxxxxxxxxxxxxxxxxxx1xxx010110x1  bgezc $t0, 0
{                    "bgezl GPREG , NUM",{0x00000304,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00011000001xx  bgezl $zero, 4
{                     "bgtz GPREG , NUM",{0x0000001C,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000000111xx  bgtz $zero, 0
{                  "bgtzalc GPREG , NUM",{0x0000011C,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx00011100  bgtzalc $at, 0
{                    "bgtzc GPREG , NUM",{0x0000015C,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx01011100  bgtzc $at, 0
{                    "bgtzl GPREG , NUM",{0x0000005C,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000010111xx  bgtzl $zero, 4
{           "binsl.b WREG , WREG , WREG",{0x0D00007B,0xC0FF1F00}}, // xx001101xxxxxxxx000xxxxx01111011  binsl.b $w0, $w0, $w0
{           "binsl.d WREG , WREG , WREG",{0x0D00607B,0xC0FF1F00}}, // xx001101xxxxxxxx011xxxxx01111011  binsl.d $w0, $w0, $w0
{           "binsl.h WREG , WREG , WREG",{0x0D00207B,0xC0FF1F00}}, // xx001101xxxxxxxx001xxxxx01111011  binsl.h $w0, $w0, $w0
{           "binsl.w WREG , WREG , WREG",{0x0D00407B,0xC0FF1F00}}, // xx001101xxxxxxxx010xxxxx01111011  binsl.w $w0, $w0, $w0
{           "binsli.b WREG , WREG , NUM",{0x0900707B,0xC0FF0700}}, // xx001001xxxxxxxx01110xxx01111011  binsli.b $w0, $w0, 0
{           "binsli.d WREG , WREG , NUM",{0x0900007B,0xC0FF3F00}}, // xx001001xxxxxxxx00xxxxxx01111011  binsli.d $w0, $w0, 0
{           "binsli.h WREG , WREG , NUM",{0x0900607B,0xC0FF0F00}}, // xx001001xxxxxxxx0110xxxx01111011  binsli.h $w0, $w0, 0
{           "binsli.w WREG , WREG , NUM",{0x0900407B,0xC0FF1F00}}, // xx001001xxxxxxxx010xxxxx01111011  binsli.w $w0, $w0, 0
{           "binsr.b WREG , WREG , WREG",{0x0D00807B,0xC0FF1F00}}, // xx001101xxxxxxxx100xxxxx01111011  binsr.b $w0, $w0, $w0
{           "binsr.d WREG , WREG , WREG",{0x0D00E07B,0xC0FF1F00}}, // xx001101xxxxxxxx111xxxxx01111011  binsr.d $w0, $w0, $w0
{           "binsr.h WREG , WREG , WREG",{0x0D00A07B,0xC0FF1F00}}, // xx001101xxxxxxxx101xxxxx01111011  binsr.h $w0, $w0, $w0
{           "binsr.w WREG , WREG , WREG",{0x0D00C07B,0xC0FF1F00}}, // xx001101xxxxxxxx110xxxxx01111011  binsr.w $w0, $w0, $w0
{           "binsri.b WREG , WREG , NUM",{0x0900F07B,0xC0FF0700}}, // xx001001xxxxxxxx11110xxx01111011  binsri.b $w0, $w0, 0
{           "binsri.d WREG , WREG , NUM",{0x0900807B,0xC0FF3F00}}, // xx001001xxxxxxxx10xxxxxx01111011  binsri.d $w0, $w0, 0
{           "binsri.h WREG , WREG , NUM",{0x0900E07B,0xC0FF0F00}}, // xx001001xxxxxxxx1110xxxx01111011  binsri.h $w0, $w0, 0
{           "binsri.w WREG , WREG , NUM",{0x0900C07B,0xC0FF1F00}}, // xx001001xxxxxxxx110xxxxx01111011  binsri.w $w0, $w0, 0
{                 "bitrev GPREG , GPREG",{0xD206007C,0x00F81F00}}, // 11010010xxxxx110000xxxxx01111100  bitrev $zero, $zero
{                "bitswap GPREG , GPREG",{0x2000007C,0x00F81F00}}, // 00100000xxxxx000000xxxxx01111100  bitswap $zero, $zero
{                     "blez GPREG , NUM",{0x00000018,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000000110xx  blez $zero, 4
{                  "blezalc GPREG , NUM",{0x00000118,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx00011000  blezalc $at, 0
{                    "blezc GPREG , NUM",{0x00000158,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx01011000  blezc $at, 0
{                    "blezl GPREG , NUM",{0x00000058,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000010110xx  blezl $zero, 4
{             "bltc GPREG , GPREG , NUM",{0x0000015D,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx010111xx  bltc $t0, $at, 0
{            "bltuc GPREG , GPREG , NUM",{0x0000011D,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000111xx  bltuc $t0, $at, 0
{                     "bltz GPREG , NUM",{0x00000004,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000000001xx  bltz $zero, 4
{                  "bltzalc GPREG , NUM",{0x0000081D,0xFFFFF702}}, // xxxxxxxxxxxxxxxxxxxx1xxx000111x1  bltzalc $t0, 0
{                  "bltzall GPREG , NUM",{0x00001204,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx10010000001xx  bltzall $zero, 4
{                    "bltzc GPREG , NUM",{0x0000085D,0xFFFFF702}}, // xxxxxxxxxxxxxxxxxxxx1xxx010111x1  bltzc $t0, 0
{                    "bltzl GPREG , NUM",{0x00000204,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00010000001xx  bltzl $zero, 4
{            "bmnz.v WREG , WREG , WREG",{0x1E008078,0xC0FF1F00}}, // xx011110xxxxxxxx100xxxxx01111000  bmnz.v $w0, $w0, $w0
{            "bmnzi.b WREG , WREG , NUM",{0x01000078,0xC0FFFF00}}, // xx000001xxxxxxxxxxxxxxxx01111000  bmnzi.b $w0, $w0, 0
{             "bmz.v WREG , WREG , WREG",{0x1E00A078,0xC0FF1F00}}, // xx011110xxxxxxxx101xxxxx01111000  bmz.v $w0, $w0, $w0
{             "bmzi.b WREG , WREG , NUM",{0x01000079,0xC0FFFF00}}, // xx000001xxxxxxxxxxxxxxxx01111001  bmzi.b $w0, $w0, 0
{              "bne GPREG , GPREG , NUM",{0x00000114,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000101xx  bne $zero, $at, 4
{             "bnec GPREG , GPREG , NUM",{0x00000961,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx011000xx  bnec $t0, $t1, 0
{            "bneg.b WREG , WREG , WREG",{0x0D00807A,0xC0FF1F00}}, // xx001101xxxxxxxx100xxxxx01111010  bneg.b $w0, $w0, $w0
{            "bneg.d WREG , WREG , WREG",{0x0D00E07A,0xC0FF1F00}}, // xx001101xxxxxxxx111xxxxx01111010  bneg.d $w0, $w0, $w0
{            "bneg.h WREG , WREG , WREG",{0x0D00A07A,0xC0FF1F00}}, // xx001101xxxxxxxx101xxxxx01111010  bneg.h $w0, $w0, $w0
{            "bneg.w WREG , WREG , WREG",{0x0D00C07A,0xC0FF1F00}}, // xx001101xxxxxxxx110xxxxx01111010  bneg.w $w0, $w0, $w0
{            "bnegi.b WREG , WREG , NUM",{0x0900F07A,0xC0FF0700}}, // xx001001xxxxxxxx11110xxx01111010  bnegi.b $w0, $w0, 0
{            "bnegi.d WREG , WREG , NUM",{0x0900807A,0xC0FF3F00}}, // xx001001xxxxxxxx10xxxxxx01111010  bnegi.d $w0, $w0, 0
{            "bnegi.h WREG , WREG , NUM",{0x0900E07A,0xC0FF0F00}}, // xx001001xxxxxxxx1110xxxx01111010  bnegi.h $w0, $w0, 0
{            "bnegi.w WREG , WREG , NUM",{0x0900C07A,0xC0FF1F00}}, // xx001001xxxxxxxx110xxxxx01111010  bnegi.w $w0, $w0, 0
{             "bnel GPREG , GPREG , NUM",{0x00000154,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx010101xx  bnel $zero, $at, 4
{                     "bnez GPREG , NUM",{0x00000014,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000000101xx  bnez $zero, 4
{                  "bnezalc GPREG , NUM",{0x00000160,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx01100000  bnezalc $at, 0
{                    "bnezc GPREG , NUM",{0x000000F9,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx111110xx  bnezc $t0, 0
{                    "bnezl GPREG , NUM",{0x00000054,0xFFFFE003}}, // xxxxxxxxxxxxxxxxxxx00000010101xx  bnezl $zero, 4
{             "bnvc GPREG , GPREG , NUM",{0x00000060,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx011000xx  bnvc $zero, $zero, 0
{                     "bnz.b WREG , NUM",{0x00008047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx100xxxxx01000111  bnz.b $w0, 4
{                     "bnz.d WREG , NUM",{0x0000E047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx111xxxxx01000111  bnz.d $w0, 4
{                     "bnz.h WREG , NUM",{0x0000A047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx101xxxxx01000111  bnz.h $w0, 4
{                     "bnz.v WREG , NUM",{0x0000E045,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx111xxxxx01000101  bnz.v $w0, 4
{                     "bnz.w WREG , NUM",{0x0000C047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx110xxxxx01000111  bnz.w $w0, 4
{             "bovc GPREG , GPREG , NUM",{0x00000020,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001000xx  bovc $zero, $zero, 0
{                         "bposge32 NUM",{0x00001C04,0xFFFF0000}}, // xxxxxxxxxxxxxxxx0001110000000100  bposge32 4
{                                "break",{0x0D000000,0x00000000}}, // 00001101000000000000000000000000  break
{                            "break NUM",{0x0D000002,0x0000FF03}}, // 0000110100000000xxxxxxxx000000xx  break 0x200
{                      "break NUM , NUM",{0x0D010000,0xC0FFFF03}}, // xx001101xxxxxxxxxxxxxxxx000000xx  break 0, 4
{            "bsel.v WREG , WREG , WREG",{0x1E00C078,0xC0FF1F00}}, // xx011110xxxxxxxx110xxxxx01111000  bsel.v $w0, $w0, $w0
{            "bseli.b WREG , WREG , NUM",{0x0100007A,0xC0FFFF00}}, // xx000001xxxxxxxxxxxxxxxx01111010  bseli.b $w0, $w0, 0
{            "bset.b WREG , WREG , WREG",{0x0D00007A,0xC0FF1F00}}, // xx001101xxxxxxxx000xxxxx01111010  bset.b $w0, $w0, $w0
{            "bset.d WREG , WREG , WREG",{0x0D00607A,0xC0FF1F00}}, // xx001101xxxxxxxx011xxxxx01111010  bset.d $w0, $w0, $w0
{            "bset.h WREG , WREG , WREG",{0x0D00207A,0xC0FF1F00}}, // xx001101xxxxxxxx001xxxxx01111010  bset.h $w0, $w0, $w0
{            "bset.w WREG , WREG , WREG",{0x0D00407A,0xC0FF1F00}}, // xx001101xxxxxxxx010xxxxx01111010  bset.w $w0, $w0, $w0
{            "bseti.b WREG , WREG , NUM",{0x0900707A,0xC0FF0700}}, // xx001001xxxxxxxx01110xxx01111010  bseti.b $w0, $w0, 0
{            "bseti.d WREG , WREG , NUM",{0x0900007A,0xC0FF3F00}}, // xx001001xxxxxxxx00xxxxxx01111010  bseti.d $w0, $w0, 0
{            "bseti.h WREG , WREG , NUM",{0x0900607A,0xC0FF0F00}}, // xx001001xxxxxxxx0110xxxx01111010  bseti.h $w0, $w0, 0
{            "bseti.w WREG , WREG , NUM",{0x0900407A,0xC0FF1F00}}, // xx001001xxxxxxxx010xxxxx01111010  bseti.w $w0, $w0, 0
{                      "bz.b WREG , NUM",{0x00000047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx01000111  bz.b $w0, 4
{                      "bz.d WREG , NUM",{0x00006047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx011xxxxx01000111  bz.d $w0, 4
{                      "bz.h WREG , NUM",{0x00002047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx001xxxxx01000111  bz.h $w0, 4
{                      "bz.v WREG , NUM",{0x00006045,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx011xxxxx01000101  bz.v $w0, 4
{                      "bz.w WREG , NUM",{0x00004047,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx010xxxxx01000111  bz.w $w0, 4
{                          "cache , ( )",{0x2500007C,0x00000000}}, // 00100101000000000000000001111100  cache 0x450, ()
{                      "cache , ( NUM )",{0x2500007E,0x80FFE003}}, // x0100101xxxxxxxxxxx00000011111xx  cache 0x450, (0x100000)
{                      "cache , NUM ( )",{0x2500017C,0x00001F00}}, // 0010010100000000000xxxxx01111100  cache 0x450, 1()
{                  "cache , NUM ( NUM )",{0x2500017E,0x80FFFF03}}, // x0100101xxxxxxxxxxxxxxxx011111xx  cache 0x450, 1(0x100000)
{                 "ceil.w.d FREG , FREG",{0x0E002046,0xC0F70000}}, // xx001110xxxx0xxx0010000001000110  ceil.w.d $f0, $f0
{                 "ceil.w.s FREG , FREG",{0x0E000046,0xC0FF0000}}, // xx001110xxxxxxxx0000000001000110  ceil.w.s $f0, $f0
{             "ceq.b WREG , WREG , WREG",{0x0F000078,0xC0FF1F00}}, // xx001111xxxxxxxx000xxxxx01111000  ceq.b $w0, $w0, $w0
{             "ceq.d WREG , WREG , WREG",{0x0F006078,0xC0FF1F00}}, // xx001111xxxxxxxx011xxxxx01111000  ceq.d $w0, $w0, $w0
{             "ceq.h WREG , WREG , WREG",{0x0F002078,0xC0FF1F00}}, // xx001111xxxxxxxx001xxxxx01111000  ceq.h $w0, $w0, $w0
{             "ceq.w WREG , WREG , WREG",{0x0F004078,0xC0FF1F00}}, // xx001111xxxxxxxx010xxxxx01111000  ceq.w $w0, $w0, $w0
{             "ceqi.b WREG , WREG , NUM",{0x07000078,0xC0FF1F00}}, // xx000111xxxxxxxx000xxxxx01111000  ceqi.b $w0, $w0, 0
{             "ceqi.d WREG , WREG , NUM",{0x07006078,0xC0FF1F00}}, // xx000111xxxxxxxx011xxxxx01111000  ceqi.d $w0, $w0, 0
{             "ceqi.h WREG , WREG , NUM",{0x07002078,0xC0FF1F00}}, // xx000111xxxxxxxx001xxxxx01111000  ceqi.h $w0, $w0, 0
{             "ceqi.w WREG , WREG , NUM",{0x07004078,0xC0FF1F00}}, // xx000111xxxxxxxx010xxxxx01111000  ceqi.w $w0, $w0, 0
{                    "cfc1 GPREG , CASH",{0x00004044,0x00F81F00}}, // 00000000xxxxx000010xxxxx01000100  cfc1 $zero, $0
{                  "cfcmsa GPREG , CASH",{0x19007E78,0xC03F0000}}, // xx01100100xxxxxx0111111001111000  cfcmsa $zero, $0
{                  "class.d FREG , FREG",{0x1B002046,0xC0FF0000}}, // xx011011xxxxxxxx0010000001000110  class.d $f0, $f0
{                  "class.s FREG , FREG",{0x1B000046,0xC0FF0000}}, // xx011011xxxxxxxx0000000001000110  class.s $f0, $f0
{           "cle_s.b WREG , WREG , WREG",{0x0F00007A,0xC0FF1F00}}, // xx001111xxxxxxxx000xxxxx01111010  cle_s.b $w0, $w0, $w0
{           "cle_s.d WREG , WREG , WREG",{0x0F00607A,0xC0FF1F00}}, // xx001111xxxxxxxx011xxxxx01111010  cle_s.d $w0, $w0, $w0
{           "cle_s.h WREG , WREG , WREG",{0x0F00207A,0xC0FF1F00}}, // xx001111xxxxxxxx001xxxxx01111010  cle_s.h $w0, $w0, $w0
{           "cle_s.w WREG , WREG , WREG",{0x0F00407A,0xC0FF1F00}}, // xx001111xxxxxxxx010xxxxx01111010  cle_s.w $w0, $w0, $w0
{           "cle_u.b WREG , WREG , WREG",{0x0F00807A,0xC0FF1F00}}, // xx001111xxxxxxxx100xxxxx01111010  cle_u.b $w0, $w0, $w0
{           "cle_u.d WREG , WREG , WREG",{0x0F00E07A,0xC0FF1F00}}, // xx001111xxxxxxxx111xxxxx01111010  cle_u.d $w0, $w0, $w0
{           "cle_u.h WREG , WREG , WREG",{0x0F00A07A,0xC0FF1F00}}, // xx001111xxxxxxxx101xxxxx01111010  cle_u.h $w0, $w0, $w0
{           "cle_u.w WREG , WREG , WREG",{0x0F00C07A,0xC0FF1F00}}, // xx001111xxxxxxxx110xxxxx01111010  cle_u.w $w0, $w0, $w0
{           "clei_s.b WREG , WREG , NUM",{0x0700007A,0xC0FF1F00}}, // xx000111xxxxxxxx000xxxxx01111010  clei_s.b $w0, $w0, 0
{           "clei_s.d WREG , WREG , NUM",{0x0700607A,0xC0FF1F00}}, // xx000111xxxxxxxx011xxxxx01111010  clei_s.d $w0, $w0, 0
{           "clei_s.h WREG , WREG , NUM",{0x0700207A,0xC0FF1F00}}, // xx000111xxxxxxxx001xxxxx01111010  clei_s.h $w0, $w0, 0
{           "clei_s.w WREG , WREG , NUM",{0x0700407A,0xC0FF1F00}}, // xx000111xxxxxxxx010xxxxx01111010  clei_s.w $w0, $w0, 0
{           "clei_u.b WREG , WREG , NUM",{0x0700807A,0xC0FF1F00}}, // xx000111xxxxxxxx100xxxxx01111010  clei_u.b $w0, $w0, 0
{           "clei_u.d WREG , WREG , NUM",{0x0700E07A,0xC0FF1F00}}, // xx000111xxxxxxxx111xxxxx01111010  clei_u.d $w0, $w0, 0
{           "clei_u.h WREG , WREG , NUM",{0x0700A07A,0xC0FF1F00}}, // xx000111xxxxxxxx101xxxxx01111010  clei_u.h $w0, $w0, 0
{           "clei_u.w WREG , WREG , NUM",{0x0700C07A,0xC0FF1F00}}, // xx000111xxxxxxxx110xxxxx01111010  clei_u.w $w0, $w0, 0
{                    "clo GPREG , GPREG",{0x51000000,0x00F8E003}}, // 01010001xxxxx000xxx00000000000xx  clo $zero, $zero
{           "clt_s.b WREG , WREG , WREG",{0x0F000079,0xC0FF1F00}}, // xx001111xxxxxxxx000xxxxx01111001  clt_s.b $w0, $w0, $w0
{           "clt_s.d WREG , WREG , WREG",{0x0F006079,0xC0FF1F00}}, // xx001111xxxxxxxx011xxxxx01111001  clt_s.d $w0, $w0, $w0
{           "clt_s.h WREG , WREG , WREG",{0x0F002079,0xC0FF1F00}}, // xx001111xxxxxxxx001xxxxx01111001  clt_s.h $w0, $w0, $w0
{           "clt_s.w WREG , WREG , WREG",{0x0F004079,0xC0FF1F00}}, // xx001111xxxxxxxx010xxxxx01111001  clt_s.w $w0, $w0, $w0
{           "clt_u.b WREG , WREG , WREG",{0x0F008079,0xC0FF1F00}}, // xx001111xxxxxxxx100xxxxx01111001  clt_u.b $w0, $w0, $w0
{           "clt_u.d WREG , WREG , WREG",{0x0F00E079,0xC0FF1F00}}, // xx001111xxxxxxxx111xxxxx01111001  clt_u.d $w0, $w0, $w0
{           "clt_u.h WREG , WREG , WREG",{0x0F00A079,0xC0FF1F00}}, // xx001111xxxxxxxx101xxxxx01111001  clt_u.h $w0, $w0, $w0
{           "clt_u.w WREG , WREG , WREG",{0x0F00C079,0xC0FF1F00}}, // xx001111xxxxxxxx110xxxxx01111001  clt_u.w $w0, $w0, $w0
{           "clti_s.b WREG , WREG , NUM",{0x07000079,0xC0FF1F00}}, // xx000111xxxxxxxx000xxxxx01111001  clti_s.b $w0, $w0, 0
{           "clti_s.d WREG , WREG , NUM",{0x07006079,0xC0FF1F00}}, // xx000111xxxxxxxx011xxxxx01111001  clti_s.d $w0, $w0, 0
{           "clti_s.h WREG , WREG , NUM",{0x07002079,0xC0FF1F00}}, // xx000111xxxxxxxx001xxxxx01111001  clti_s.h $w0, $w0, 0
{           "clti_s.w WREG , WREG , NUM",{0x07004079,0xC0FF1F00}}, // xx000111xxxxxxxx010xxxxx01111001  clti_s.w $w0, $w0, 0
{           "clti_u.b WREG , WREG , NUM",{0x07008079,0xC0FF1F00}}, // xx000111xxxxxxxx100xxxxx01111001  clti_u.b $w0, $w0, 0
{           "clti_u.d WREG , WREG , NUM",{0x0700E079,0xC0FF1F00}}, // xx000111xxxxxxxx111xxxxx01111001  clti_u.d $w0, $w0, 0
{           "clti_u.h WREG , WREG , NUM",{0x0700A079,0xC0FF1F00}}, // xx000111xxxxxxxx101xxxxx01111001  clti_u.h $w0, $w0, 0
{           "clti_u.w WREG , WREG , NUM",{0x0700C079,0xC0FF1F00}}, // xx000111xxxxxxxx110xxxxx01111001  clti_u.w $w0, $w0, 0
{                    "clz GPREG , GPREG",{0x50000000,0x00F8E003}}, // 01010000xxxxx000xxx00000000000xx  clz $zero, $zero
{          "cmp.af.d FREG , FREG , FREG",{0x0000A046,0xC0FF1F00}}, // xx000000xxxxxxxx101xxxxx01000110  cmp.af.d $f0, $f0, $f0
{          "cmp.af.s FREG , FREG , FREG",{0x00008046,0xC0FF1F00}}, // xx000000xxxxxxxx100xxxxx01000110  cmp.af.s $f0, $f0, $f0
{          "cmp.eq.d FREG , FREG , FREG",{0x0200A046,0xC0FF1F00}}, // xx000010xxxxxxxx101xxxxx01000110  cmp.eq.d $f0, $f0, $f0
{              "cmp.eq.ph GPREG , GPREG",{0x1102007C,0x0000FF03}}, // 0001000100000010xxxxxxxx011111xx  cmp.eq.ph $zero, $zero
{          "cmp.eq.s FREG , FREG , FREG",{0x02008046,0xC0FF1F00}}, // xx000010xxxxxxxx100xxxxx01000110  cmp.eq.s $f0, $f0, $f0
{          "cmp.le.d FREG , FREG , FREG",{0x0600A046,0xC0FF1F00}}, // xx000110xxxxxxxx101xxxxx01000110  cmp.le.d $f0, $f0, $f0
{              "cmp.le.ph GPREG , GPREG",{0x9102007C,0x0000FF03}}, // 1001000100000010xxxxxxxx011111xx  cmp.le.ph $zero, $zero
{          "cmp.le.s FREG , FREG , FREG",{0x06008046,0xC0FF1F00}}, // xx000110xxxxxxxx100xxxxx01000110  cmp.le.s $f0, $f0, $f0
{          "cmp.lt.d FREG , FREG , FREG",{0x0400A046,0xC0FF1F00}}, // xx000100xxxxxxxx101xxxxx01000110  cmp.lt.d $f0, $f0, $f0
{              "cmp.lt.ph GPREG , GPREG",{0x5102007C,0x0000FF03}}, // 0101000100000010xxxxxxxx011111xx  cmp.lt.ph $zero, $zero
{          "cmp.lt.s FREG , FREG , FREG",{0x04008046,0xC0FF1F00}}, // xx000100xxxxxxxx100xxxxx01000110  cmp.lt.s $f0, $f0, $f0
{         "cmp.saf.d FREG , FREG , FREG",{0x0800A046,0xC0FF1F00}}, // xx001000xxxxxxxx101xxxxx01000110  cmp.saf.d $f0, $f0, $f0
{         "cmp.saf.s FREG , FREG , FREG",{0x08008046,0xC0FF1F00}}, // xx001000xxxxxxxx100xxxxx01000110  cmp.saf.s $f0, $f0, $f0
{         "cmp.seq.d FREG , FREG , FREG",{0x0A00A046,0xC0FF1F00}}, // xx001010xxxxxxxx101xxxxx01000110  cmp.seq.d $f0, $f0, $f0
{         "cmp.seq.s FREG , FREG , FREG",{0x0A008046,0xC0FF1F00}}, // xx001010xxxxxxxx100xxxxx01000110  cmp.seq.s $f0, $f0, $f0
{         "cmp.sle.d FREG , FREG , FREG",{0x0E00A046,0xC0FF1F00}}, // xx001110xxxxxxxx101xxxxx01000110  cmp.sle.d $f0, $f0, $f0
{         "cmp.sle.s FREG , FREG , FREG",{0x0E008046,0xC0FF1F00}}, // xx001110xxxxxxxx100xxxxx01000110  cmp.sle.s $f0, $f0, $f0
{         "cmp.slt.d FREG , FREG , FREG",{0x0C00A046,0xC0FF1F00}}, // xx001100xxxxxxxx101xxxxx01000110  cmp.slt.d $f0, $f0, $f0
{         "cmp.slt.s FREG , FREG , FREG",{0x0C008046,0xC0FF1F00}}, // xx001100xxxxxxxx100xxxxx01000110  cmp.slt.s $f0, $f0, $f0
{        "cmp.sueq.d FREG , FREG , FREG",{0x0B00A046,0xC0FF1F00}}, // xx001011xxxxxxxx101xxxxx01000110  cmp.sueq.d $f0, $f0, $f0
{        "cmp.sueq.s FREG , FREG , FREG",{0x0B008046,0xC0FF1F00}}, // xx001011xxxxxxxx100xxxxx01000110  cmp.sueq.s $f0, $f0, $f0
{        "cmp.sule.d FREG , FREG , FREG",{0x0F00A046,0xC0FF1F00}}, // xx001111xxxxxxxx101xxxxx01000110  cmp.sule.d $f0, $f0, $f0
{        "cmp.sule.s FREG , FREG , FREG",{0x0F008046,0xC0FF1F00}}, // xx001111xxxxxxxx100xxxxx01000110  cmp.sule.s $f0, $f0, $f0
{        "cmp.sult.d FREG , FREG , FREG",{0x0D00A046,0xC0FF1F00}}, // xx001101xxxxxxxx101xxxxx01000110  cmp.sult.d $f0, $f0, $f0
{        "cmp.sult.s FREG , FREG , FREG",{0x0D008046,0xC0FF1F00}}, // xx001101xxxxxxxx100xxxxx01000110  cmp.sult.s $f0, $f0, $f0
{         "cmp.sun.d FREG , FREG , FREG",{0x0900A046,0xC0FF1F00}}, // xx001001xxxxxxxx101xxxxx01000110  cmp.sun.d $f0, $f0, $f0
{         "cmp.sun.s FREG , FREG , FREG",{0x09008046,0xC0FF1F00}}, // xx001001xxxxxxxx100xxxxx01000110  cmp.sun.s $f0, $f0, $f0
{         "cmp.ueq.d FREG , FREG , FREG",{0x0300A046,0xC0FF1F00}}, // xx000011xxxxxxxx101xxxxx01000110  cmp.ueq.d $f0, $f0, $f0
{         "cmp.ueq.s FREG , FREG , FREG",{0x03008046,0xC0FF1F00}}, // xx000011xxxxxxxx100xxxxx01000110  cmp.ueq.s $f0, $f0, $f0
{         "cmp.ule.d FREG , FREG , FREG",{0x0700A046,0xC0FF1F00}}, // xx000111xxxxxxxx101xxxxx01000110  cmp.ule.d $f0, $f0, $f0
{         "cmp.ule.s FREG , FREG , FREG",{0x07008046,0xC0FF1F00}}, // xx000111xxxxxxxx100xxxxx01000110  cmp.ule.s $f0, $f0, $f0
{         "cmp.ult.d FREG , FREG , FREG",{0x0500A046,0xC0FF1F00}}, // xx000101xxxxxxxx101xxxxx01000110  cmp.ult.d $f0, $f0, $f0
{         "cmp.ult.s FREG , FREG , FREG",{0x05008046,0xC0FF1F00}}, // xx000101xxxxxxxx100xxxxx01000110  cmp.ult.s $f0, $f0, $f0
{          "cmp.un.d FREG , FREG , FREG",{0x0100A046,0xC0FF1F00}}, // xx000001xxxxxxxx101xxxxx01000110  cmp.un.d $f0, $f0, $f0
{          "cmp.un.s FREG , FREG , FREG",{0x01008046,0xC0FF1F00}}, // xx000001xxxxxxxx100xxxxx01000110  cmp.un.s $f0, $f0, $f0
{   "cmpgdu.eq.qb GPREG , GPREG , GPREG",{0x1106007C,0x00F8FF03}}, // 00010001xxxxx110xxxxxxxx011111xx  cmpgdu.eq.qb $zero, $zero, $zero
{   "cmpgdu.le.qb GPREG , GPREG , GPREG",{0x9106007C,0x00F8FF03}}, // 10010001xxxxx110xxxxxxxx011111xx  cmpgdu.le.qb $zero, $zero, $zero
{   "cmpgdu.lt.qb GPREG , GPREG , GPREG",{0x5106007C,0x00F8FF03}}, // 01010001xxxxx110xxxxxxxx011111xx  cmpgdu.lt.qb $zero, $zero, $zero
{    "cmpgu.eq.qb GPREG , GPREG , GPREG",{0x1101007C,0x00F8FF03}}, // 00010001xxxxx001xxxxxxxx011111xx  cmpgu.eq.qb $zero, $zero, $zero
{    "cmpgu.le.qb GPREG , GPREG , GPREG",{0x9101007C,0x00F8FF03}}, // 10010001xxxxx001xxxxxxxx011111xx  cmpgu.le.qb $zero, $zero, $zero
{    "cmpgu.lt.qb GPREG , GPREG , GPREG",{0x5101007C,0x00F8FF03}}, // 01010001xxxxx001xxxxxxxx011111xx  cmpgu.lt.qb $zero, $zero, $zero
{             "cmpu.eq.qb GPREG , GPREG",{0x1100007C,0x0000FF03}}, // 0001000100000000xxxxxxxx011111xx  cmpu.eq.qb $zero, $zero
{             "cmpu.le.qb GPREG , GPREG",{0x9100007C,0x0000FF03}}, // 1001000100000000xxxxxxxx011111xx  cmpu.le.qb $zero, $zero
{             "cmpu.lt.qb GPREG , GPREG",{0x5100007C,0x0000FF03}}, // 0101000100000000xxxxxxxx011111xx  cmpu.lt.qb $zero, $zero
{        "copy_s.b GPREG , WREG [ NUM ]",{0x19008078,0xC0FF0F00}}, // xx011001xxxxxxxx1000xxxx01111000  copy_s.b $zero, $w0[0]
{        "copy_s.d GPREG , WREG [ NUM ]",{0x1900B878,0xC0FF0100}}, // xx011001xxxxxxxx1011100x01111000  copy_s.d $zero, $w0[0]
{        "copy_s.h GPREG , WREG [ NUM ]",{0x1900A078,0xC0FF0700}}, // xx011001xxxxxxxx10100xxx01111000  copy_s.h $zero, $w0[0]
{        "copy_s.w GPREG , WREG [ NUM ]",{0x1900B078,0xC0FF0300}}, // xx011001xxxxxxxx101100xx01111000  copy_s.w $zero, $w0[0]
{        "copy_u.b GPREG , WREG [ NUM ]",{0x1900C078,0xC0FF0F00}}, // xx011001xxxxxxxx1100xxxx01111000  copy_u.b $zero, $w0[0]
{        "copy_u.d GPREG , WREG [ NUM ]",{0x1900F878,0xC0FF0100}}, // xx011001xxxxxxxx1111100x01111000  copy_u.d $zero, $w0[0]
{        "copy_u.h GPREG , WREG [ NUM ]",{0x1900E078,0xC0FF0700}}, // xx011001xxxxxxxx11100xxx01111000  copy_u.h $zero, $w0[0]
{        "copy_u.w GPREG , WREG [ NUM ]",{0x1900F078,0xC0FF0300}}, // xx011001xxxxxxxx111100xx01111000  copy_u.w $zero, $w0[0]
{                    "ctc1 GPREG , CASH",{0x0000C044,0x00F81F00}}, // 00000000xxxxx000110xxxxx01000100  ctc1 $zero, $0
{                  "ctcmsa CASH , GPREG",{0x19003E78,0xC0F90000}}, // xx011001xxxxx00x0011111001111000  ctcmsa $0, $zero
{                  "cvt.d.s FREG , FREG",{0x21000046,0x80FF0000}}, // x0100001xxxxxxxx0000000001000110  cvt.d.s $f0, $f0
{                  "cvt.d.w FREG , FREG",{0x21008046,0x80FF0000}}, // x0100001xxxxxxxx1000000001000110  cvt.d.w $f0, $f0
{                  "cvt.l.d FREG , FREG",{0x25002046,0xC0FF0000}}, // xx100101xxxxxxxx0010000001000110  cvt.l.d $f0, $f0
{                  "cvt.l.s FREG , FREG",{0x25000046,0xC0FF0000}}, // xx100101xxxxxxxx0000000001000110  cvt.l.s $f0, $f0
{                  "cvt.s.d FREG , FREG",{0x20002046,0xC0F70000}}, // xx100000xxxx0xxx0010000001000110  cvt.s.d $f0, $f0
{                  "cvt.s.w FREG , FREG",{0x20008046,0xC0FF0000}}, // xx100000xxxxxxxx1000000001000110  cvt.s.w $f0, $f0
{                  "cvt.w.d FREG , FREG",{0x24002046,0xC0F70000}}, // xx100100xxxx0xxx0010000001000110  cvt.w.d $f0, $f0
{                  "cvt.w.s FREG , FREG",{0x24000046,0xC0FF0000}}, // xx100100xxxxxxxx0000000001000110  cvt.w.s $f0, $f0
{                                "deret",{0x1F000042,0x00000000}}, // 00011111000000000000000001000010  deret
{                                   "di",{0x00606041,0x00000000}}, // 00000000011000000110000001000001  di
{                             "di GPREG",{0x00606141,0x00001F00}}, // 0000000001100000011xxxxx01000001  di $at
{            "div GPREG , GPREG , GPREG",{0x9A000000,0x00F8FF03}}, // 10011010xxxxx000xxxxxxxx000000xx  div $zero, $zero, $zero
{             "div.d FREG , FREG , FREG",{0x03002046,0x80F71E00}}, // x0000011xxxx0xxx001xxxx001000110  div.d $f0, $f0, $f0
{             "div.s FREG , FREG , FREG",{0x03000046,0xC0FF1F00}}, // xx000011xxxxxxxx000xxxxx01000110  div.s $f0, $f0, $f0
{           "div_s.b WREG , WREG , WREG",{0x1200007A,0xC0FF1F00}}, // xx010010xxxxxxxx000xxxxx01111010  div_s.b $w0, $w0, $w0
{           "div_s.d WREG , WREG , WREG",{0x1200607A,0xC0FF1F00}}, // xx010010xxxxxxxx011xxxxx01111010  div_s.d $w0, $w0, $w0
{           "div_s.h WREG , WREG , WREG",{0x1200207A,0xC0FF1F00}}, // xx010010xxxxxxxx001xxxxx01111010  div_s.h $w0, $w0, $w0
{           "div_s.w WREG , WREG , WREG",{0x1200407A,0xC0FF1F00}}, // xx010010xxxxxxxx010xxxxx01111010  div_s.w $w0, $w0, $w0
{           "div_u.b WREG , WREG , WREG",{0x1200807A,0xC0FF1F00}}, // xx010010xxxxxxxx100xxxxx01111010  div_u.b $w0, $w0, $w0
{           "div_u.d WREG , WREG , WREG",{0x1200E07A,0xC0FF1F00}}, // xx010010xxxxxxxx111xxxxx01111010  div_u.d $w0, $w0, $w0
{           "div_u.h WREG , WREG , WREG",{0x1200A07A,0xC0FF1F00}}, // xx010010xxxxxxxx101xxxxx01111010  div_u.h $w0, $w0, $w0
{           "div_u.w WREG , WREG , WREG",{0x1200C07A,0xC0FF1F00}}, // xx010010xxxxxxxx110xxxxx01111010  div_u.w $w0, $w0, $w0
{           "divu GPREG , GPREG , GPREG",{0x9B000000,0x00F8FF03}}, // 10011011xxxxx000xxxxxxxx000000xx  divu $zero, $zero, $zero
{     "dlsa GPREG , GPREG , GPREG , NUM",{0x15000000,0xC0F8FF03}}, // xx010101xxxxx000xxxxxxxx000000xx  dlsa $zero, $zero, $zero, 1
{                   "dmfc1 GPREG , FREG",{0x00002044,0x00F81F00}}, // 00000000xxxxx000001xxxxx01000100  dmfc1 $zero, $f0
{                   "dmtc1 GPREG , FREG",{0x0000A044,0x00F81F00}}, // 00000000xxxxx000101xxxxx01000100  dmtc1 $zero, $f0
{          "dotp_s.d WREG , WREG , WREG",{0x13006078,0xC0FF1F00}}, // xx010011xxxxxxxx011xxxxx01111000  dotp_s.d $w0, $w0, $w0
{          "dotp_s.h WREG , WREG , WREG",{0x13002078,0xC0FF1F00}}, // xx010011xxxxxxxx001xxxxx01111000  dotp_s.h $w0, $w0, $w0
{          "dotp_s.w WREG , WREG , WREG",{0x13004078,0xC0FF1F00}}, // xx010011xxxxxxxx010xxxxx01111000  dotp_s.w $w0, $w0, $w0
{          "dotp_u.d WREG , WREG , WREG",{0x1300E078,0xC0FF1F00}}, // xx010011xxxxxxxx111xxxxx01111000  dotp_u.d $w0, $w0, $w0
{          "dotp_u.h WREG , WREG , WREG",{0x1300A078,0xC0FF1F00}}, // xx010011xxxxxxxx101xxxxx01111000  dotp_u.h $w0, $w0, $w0
{          "dotp_u.w WREG , WREG , WREG",{0x1300C078,0xC0FF1F00}}, // xx010011xxxxxxxx110xxxxx01111000  dotp_u.w $w0, $w0, $w0
{       "dpa.w.ph ACREG , GPREG , GPREG",{0x3000007C,0x0018FF03}}, // 00110000000xx000xxxxxxxx011111xx  dpa.w.ph $ac0, $zero, $zero
{         "dpadd_s.d WREG , WREG , WREG",{0x13006079,0xC0FF1F00}}, // xx010011xxxxxxxx011xxxxx01111001  dpadd_s.d $w0, $w0, $w0
{         "dpadd_s.h WREG , WREG , WREG",{0x13002079,0xC0FF1F00}}, // xx010011xxxxxxxx001xxxxx01111001  dpadd_s.h $w0, $w0, $w0
{         "dpadd_s.w WREG , WREG , WREG",{0x13004079,0xC0FF1F00}}, // xx010011xxxxxxxx010xxxxx01111001  dpadd_s.w $w0, $w0, $w0
{         "dpadd_u.d WREG , WREG , WREG",{0x1300E079,0xC0FF1F00}}, // xx010011xxxxxxxx111xxxxx01111001  dpadd_u.d $w0, $w0, $w0
{         "dpadd_u.h WREG , WREG , WREG",{0x1300A079,0xC0FF1F00}}, // xx010011xxxxxxxx101xxxxx01111001  dpadd_u.h $w0, $w0, $w0
{         "dpadd_u.w WREG , WREG , WREG",{0x1300C079,0xC0FF1F00}}, // xx010011xxxxxxxx110xxxxx01111001  dpadd_u.w $w0, $w0, $w0
{    "dpaq_s.w.ph ACREG , GPREG , GPREG",{0x3001007C,0x0018FF03}}, // 00110000000xx001xxxxxxxx011111xx  dpaq_s.w.ph $ac0, $zero, $zero
{    "dpaq_sa.l.w ACREG , GPREG , GPREG",{0x3003007C,0x0018FF03}}, // 00110000000xx011xxxxxxxx011111xx  dpaq_sa.l.w $ac0, $zero, $zero
{   "dpaqx_s.w.ph ACREG , GPREG , GPREG",{0x3006007C,0x0018FF03}}, // 00110000000xx110xxxxxxxx011111xx  dpaqx_s.w.ph $ac0, $zero, $zero
{  "dpaqx_sa.w.ph ACREG , GPREG , GPREG",{0xB006007C,0x0018FF03}}, // 10110000000xx110xxxxxxxx011111xx  dpaqx_sa.w.ph $ac0, $zero, $zero
{     "dpau.h.qbl ACREG , GPREG , GPREG",{0xF000007C,0x0018FF03}}, // 11110000000xx000xxxxxxxx011111xx  dpau.h.qbl $ac0, $zero, $zero
{     "dpau.h.qbr ACREG , GPREG , GPREG",{0xF001007C,0x0018FF03}}, // 11110000000xx001xxxxxxxx011111xx  dpau.h.qbr $ac0, $zero, $zero
{      "dpax.w.ph ACREG , GPREG , GPREG",{0x3002007C,0x0018FF03}}, // 00110000000xx010xxxxxxxx011111xx  dpax.w.ph $ac0, $zero, $zero
{       "dps.w.ph ACREG , GPREG , GPREG",{0x7000007C,0x0018FF03}}, // 01110000000xx000xxxxxxxx011111xx  dps.w.ph $ac0, $zero, $zero
{    "dpsq_s.w.ph ACREG , GPREG , GPREG",{0x7001007C,0x0018FF03}}, // 01110000000xx001xxxxxxxx011111xx  dpsq_s.w.ph $ac0, $zero, $zero
{    "dpsq_sa.l.w ACREG , GPREG , GPREG",{0x7003007C,0x0018FF03}}, // 01110000000xx011xxxxxxxx011111xx  dpsq_sa.l.w $ac0, $zero, $zero
{   "dpsqx_s.w.ph ACREG , GPREG , GPREG",{0x7006007C,0x0018FF03}}, // 01110000000xx110xxxxxxxx011111xx  dpsqx_s.w.ph $ac0, $zero, $zero
{  "dpsqx_sa.w.ph ACREG , GPREG , GPREG",{0xF006007C,0x0018FF03}}, // 11110000000xx110xxxxxxxx011111xx  dpsqx_sa.w.ph $ac0, $zero, $zero
{     "dpsu.h.qbl ACREG , GPREG , GPREG",{0xF002007C,0x0018FF03}}, // 11110000000xx010xxxxxxxx011111xx  dpsu.h.qbl $ac0, $zero, $zero
{     "dpsu.h.qbr ACREG , GPREG , GPREG",{0xF003007C,0x0018FF03}}, // 11110000000xx011xxxxxxxx011111xx  dpsu.h.qbr $ac0, $zero, $zero
{         "dpsub_s.d WREG , WREG , WREG",{0x1300607A,0xC0FF1F00}}, // xx010011xxxxxxxx011xxxxx01111010  dpsub_s.d $w0, $w0, $w0
{         "dpsub_s.h WREG , WREG , WREG",{0x1300207A,0xC0FF1F00}}, // xx010011xxxxxxxx001xxxxx01111010  dpsub_s.h $w0, $w0, $w0
{         "dpsub_s.w WREG , WREG , WREG",{0x1300407A,0xC0FF1F00}}, // xx010011xxxxxxxx010xxxxx01111010  dpsub_s.w $w0, $w0, $w0
{         "dpsub_u.d WREG , WREG , WREG",{0x1300E07A,0xC0FF1F00}}, // xx010011xxxxxxxx111xxxxx01111010  dpsub_u.d $w0, $w0, $w0
{         "dpsub_u.h WREG , WREG , WREG",{0x1300A07A,0xC0FF1F00}}, // xx010011xxxxxxxx101xxxxx01111010  dpsub_u.h $w0, $w0, $w0
{         "dpsub_u.w WREG , WREG , WREG",{0x1300C07A,0xC0FF1F00}}, // xx010011xxxxxxxx110xxxxx01111010  dpsub_u.w $w0, $w0, $w0
{      "dpsx.w.ph ACREG , GPREG , GPREG",{0x7002007C,0x0018FF03}}, // 01110000000xx010xxxxxxxx011111xx  dpsx.w.ph $ac0, $zero, $zero
{                                  "ehb",{0xC0000000,0x00000000}}, // 11000000000000000000000000000000  ehb
{                                   "ei",{0x20606041,0x00000000}}, // 00100000011000000110000001000001  ei
{                             "ei GPREG",{0x20606141,0x00001F00}}, // 0010000001100000011xxxxx01000001  ei $at
{                                 "eret",{0x18000042,0x00000000}}, // 00011000000000000000000001000010  eret
{        "ext GPREG , GPREG , NUM , NUM",{0x0000007C,0xC0FFFF03}}, // xx000000xxxxxxxxxxxxxxxx011111xx  ext $zero, $zero, 0, 1
{             "extp GPREG , ACREG , NUM",{0xB800007C,0x0018FF03}}, // 10111000000xx000xxxxxxxx011111xx  extp $zero, $ac0, 0
{           "extpdp GPREG , ACREG , NUM",{0xB802007C,0x0018FF03}}, // 10111000000xx010xxxxxxxx011111xx  extpdp $zero, $ac0, 0
{        "extpdpv GPREG , ACREG , GPREG",{0xF802007C,0x0018FF03}}, // 11111000000xx010xxxxxxxx011111xx  extpdpv $zero, $ac0, $zero
{          "extpv GPREG , ACREG , GPREG",{0xF800007C,0x0018FF03}}, // 11111000000xx000xxxxxxxx011111xx  extpv $zero, $ac0, $zero
{           "extr.w GPREG , ACREG , NUM",{0x3800007C,0x0018FF03}}, // 00111000000xx000xxxxxxxx011111xx  extr.w $zero, $ac0, 0
{         "extr_r.w GPREG , ACREG , NUM",{0x3801007C,0x0018FF03}}, // 00111000000xx001xxxxxxxx011111xx  extr_r.w $zero, $ac0, 0
{        "extr_rs.w GPREG , ACREG , NUM",{0xB801007C,0x0018FF03}}, // 10111000000xx001xxxxxxxx011111xx  extr_rs.w $zero, $ac0, 0
{         "extr_s.h GPREG , ACREG , NUM",{0xB803007C,0x0018FF03}}, // 10111000000xx011xxxxxxxx011111xx  extr_s.h $zero, $ac0, 0
{        "extrv.w GPREG , ACREG , GPREG",{0x7800007C,0x0018FF03}}, // 01111000000xx000xxxxxxxx011111xx  extrv.w $zero, $ac0, $zero
{      "extrv_r.w GPREG , ACREG , GPREG",{0x7801007C,0x0018FF03}}, // 01111000000xx001xxxxxxxx011111xx  extrv_r.w $zero, $ac0, $zero
{     "extrv_rs.w GPREG , ACREG , GPREG",{0xF801007C,0x0018FF03}}, // 11111000000xx001xxxxxxxx011111xx  extrv_rs.w $zero, $ac0, $zero
{      "extrv_s.h GPREG , ACREG , GPREG",{0xF803007C,0x0018FF03}}, // 11111000000xx011xxxxxxxx011111xx  extrv_s.h $zero, $ac0, $zero
{            "fadd.d WREG , WREG , WREG",{0x1B002078,0xC0FF1F00}}, // xx011011xxxxxxxx001xxxxx01111000  fadd.d $w0, $w0, $w0
{            "fadd.w WREG , WREG , WREG",{0x1B000078,0xC0FF1F00}}, // xx011011xxxxxxxx000xxxxx01111000  fadd.w $w0, $w0, $w0
{            "fcaf.d WREG , WREG , WREG",{0x1A002078,0xC0FF1F00}}, // xx011010xxxxxxxx001xxxxx01111000  fcaf.d $w0, $w0, $w0
{            "fcaf.w WREG , WREG , WREG",{0x1A000078,0xC0FF1F00}}, // xx011010xxxxxxxx000xxxxx01111000  fcaf.w $w0, $w0, $w0
{            "fceq.d WREG , WREG , WREG",{0x1A00A078,0xC0FF1F00}}, // xx011010xxxxxxxx101xxxxx01111000  fceq.d $w0, $w0, $w0
{            "fceq.w WREG , WREG , WREG",{0x1A008078,0xC0FF1F00}}, // xx011010xxxxxxxx100xxxxx01111000  fceq.w $w0, $w0, $w0
{                 "fclass.d WREG , WREG",{0x1E00217B,0xC0FF0000}}, // xx011110xxxxxxxx0010000101111011  fclass.d $w0, $w0
{                 "fclass.w WREG , WREG",{0x1E00207B,0xC0FF0000}}, // xx011110xxxxxxxx0010000001111011  fclass.w $w0, $w0
{            "fcle.d WREG , WREG , WREG",{0x1A00A079,0xC0FF1F00}}, // xx011010xxxxxxxx101xxxxx01111001  fcle.d $w0, $w0, $w0
{            "fcle.w WREG , WREG , WREG",{0x1A008079,0xC0FF1F00}}, // xx011010xxxxxxxx100xxxxx01111001  fcle.w $w0, $w0, $w0
{            "fclt.d WREG , WREG , WREG",{0x1A002079,0xC0FF1F00}}, // xx011010xxxxxxxx001xxxxx01111001  fclt.d $w0, $w0, $w0
{            "fclt.w WREG , WREG , WREG",{0x1A000079,0xC0FF1F00}}, // xx011010xxxxxxxx000xxxxx01111001  fclt.w $w0, $w0, $w0
{            "fcne.d WREG , WREG , WREG",{0x1C00E078,0xC0FF1F00}}, // xx011100xxxxxxxx111xxxxx01111000  fcne.d $w0, $w0, $w0
{            "fcne.w WREG , WREG , WREG",{0x1C00C078,0xC0FF1F00}}, // xx011100xxxxxxxx110xxxxx01111000  fcne.w $w0, $w0, $w0
{            "fcor.d WREG , WREG , WREG",{0x1C006078,0xC0FF1F00}}, // xx011100xxxxxxxx011xxxxx01111000  fcor.d $w0, $w0, $w0
{            "fcor.w WREG , WREG , WREG",{0x1C004078,0xC0FF1F00}}, // xx011100xxxxxxxx010xxxxx01111000  fcor.w $w0, $w0, $w0
{           "fcueq.d WREG , WREG , WREG",{0x1A00E078,0xC0FF1F00}}, // xx011010xxxxxxxx111xxxxx01111000  fcueq.d $w0, $w0, $w0
{           "fcueq.w WREG , WREG , WREG",{0x1A00C078,0xC0FF1F00}}, // xx011010xxxxxxxx110xxxxx01111000  fcueq.w $w0, $w0, $w0
{           "fcule.d WREG , WREG , WREG",{0x1A00E079,0xC0FF1F00}}, // xx011010xxxxxxxx111xxxxx01111001  fcule.d $w0, $w0, $w0
{           "fcule.w WREG , WREG , WREG",{0x1A00C079,0xC0FF1F00}}, // xx011010xxxxxxxx110xxxxx01111001  fcule.w $w0, $w0, $w0
{           "fcult.d WREG , WREG , WREG",{0x1A006079,0xC0FF1F00}}, // xx011010xxxxxxxx011xxxxx01111001  fcult.d $w0, $w0, $w0
{           "fcult.w WREG , WREG , WREG",{0x1A004079,0xC0FF1F00}}, // xx011010xxxxxxxx010xxxxx01111001  fcult.w $w0, $w0, $w0
{            "fcun.d WREG , WREG , WREG",{0x1A006078,0xC0FF1F00}}, // xx011010xxxxxxxx011xxxxx01111000  fcun.d $w0, $w0, $w0
{            "fcun.w WREG , WREG , WREG",{0x1A004078,0xC0FF1F00}}, // xx011010xxxxxxxx010xxxxx01111000  fcun.w $w0, $w0, $w0
{           "fcune.d WREG , WREG , WREG",{0x1C00A078,0xC0FF1F00}}, // xx011100xxxxxxxx101xxxxx01111000  fcune.d $w0, $w0, $w0
{           "fcune.w WREG , WREG , WREG",{0x1C008078,0xC0FF1F00}}, // xx011100xxxxxxxx100xxxxx01111000  fcune.w $w0, $w0, $w0
{            "fdiv.d WREG , WREG , WREG",{0x1B00E078,0xC0FF1F00}}, // xx011011xxxxxxxx111xxxxx01111000  fdiv.d $w0, $w0, $w0
{            "fdiv.w WREG , WREG , WREG",{0x1B00C078,0xC0FF1F00}}, // xx011011xxxxxxxx110xxxxx01111000  fdiv.w $w0, $w0, $w0
{           "fexdo.h WREG , WREG , WREG",{0x1B00007A,0xC0FF1F00}}, // xx011011xxxxxxxx000xxxxx01111010  fexdo.h $w0, $w0, $w0
{           "fexdo.w WREG , WREG , WREG",{0x1B00207A,0xC0FF1F00}}, // xx011011xxxxxxxx001xxxxx01111010  fexdo.w $w0, $w0, $w0
{           "fexp2.d WREG , WREG , WREG",{0x1B00E079,0xC0FF1F00}}, // xx011011xxxxxxxx111xxxxx01111001  fexp2.d $w0, $w0, $w0
{           "fexp2.w WREG , WREG , WREG",{0x1B00C079,0xC0FF1F00}}, // xx011011xxxxxxxx110xxxxx01111001  fexp2.w $w0, $w0, $w0
{                 "fexupl.d WREG , WREG",{0x1E00317B,0xC0FF0000}}, // xx011110xxxxxxxx0011000101111011  fexupl.d $w0, $w0
{                 "fexupl.w WREG , WREG",{0x1E00307B,0xC0FF0000}}, // xx011110xxxxxxxx0011000001111011  fexupl.w $w0, $w0
{                 "fexupr.d WREG , WREG",{0x1E00337B,0xC0FF0000}}, // xx011110xxxxxxxx0011001101111011  fexupr.d $w0, $w0
{                 "fexupr.w WREG , WREG",{0x1E00327B,0xC0FF0000}}, // xx011110xxxxxxxx0011001001111011  fexupr.w $w0, $w0
{                "ffint_s.d WREG , WREG",{0x1E003D7B,0xC0FF0000}}, // xx011110xxxxxxxx0011110101111011  ffint_s.d $w0, $w0
{                "ffint_s.w WREG , WREG",{0x1E003C7B,0xC0FF0000}}, // xx011110xxxxxxxx0011110001111011  ffint_s.w $w0, $w0
{                "ffint_u.d WREG , WREG",{0x1E003F7B,0xC0FF0000}}, // xx011110xxxxxxxx0011111101111011  ffint_u.d $w0, $w0
{                "ffint_u.w WREG , WREG",{0x1E003E7B,0xC0FF0000}}, // xx011110xxxxxxxx0011111001111011  ffint_u.w $w0, $w0
{                   "ffql.d WREG , WREG",{0x1E00357B,0xC0FF0000}}, // xx011110xxxxxxxx0011010101111011  ffql.d $w0, $w0
{                   "ffql.w WREG , WREG",{0x1E00347B,0xC0FF0000}}, // xx011110xxxxxxxx0011010001111011  ffql.w $w0, $w0
{                   "ffqr.d WREG , WREG",{0x1E00377B,0xC0FF0000}}, // xx011110xxxxxxxx0011011101111011  ffqr.d $w0, $w0
{                   "ffqr.w WREG , WREG",{0x1E00367B,0xC0FF0000}}, // xx011110xxxxxxxx0011011001111011  ffqr.w $w0, $w0
{                  "fill.b WREG , GPREG",{0x1E00007B,0xC0FF0000}}, // xx011110xxxxxxxx0000000001111011  fill.b $w0, $zero
{                  "fill.d WREG , GPREG",{0x1E00037B,0xC0FF0000}}, // xx011110xxxxxxxx0000001101111011  fill.d $w0, $zero
{                  "fill.h WREG , GPREG",{0x1E00017B,0xC0FF0000}}, // xx011110xxxxxxxx0000000101111011  fill.h $w0, $zero
{                  "fill.w WREG , GPREG",{0x1E00027B,0xC0FF0000}}, // xx011110xxxxxxxx0000001001111011  fill.w $w0, $zero
{                  "flog2.d WREG , WREG",{0x1E002F7B,0xC0FF0000}}, // xx011110xxxxxxxx0010111101111011  flog2.d $w0, $w0
{                  "flog2.w WREG , WREG",{0x1E002E7B,0xC0FF0000}}, // xx011110xxxxxxxx0010111001111011  flog2.w $w0, $w0
{                "floor.w.d FREG , FREG",{0x0F002046,0xC0F70000}}, // xx001111xxxx0xxx0010000001000110  floor.w.d $f0, $f0
{                "floor.w.s FREG , FREG",{0x0F000046,0xC0FF0000}}, // xx001111xxxxxxxx0000000001000110  floor.w.s $f0, $f0
{           "fmadd.d WREG , WREG , WREG",{0x1B002079,0xC0FF1F00}}, // xx011011xxxxxxxx001xxxxx01111001  fmadd.d $w0, $w0, $w0
{           "fmadd.w WREG , WREG , WREG",{0x1B000079,0xC0FF1F00}}, // xx011011xxxxxxxx000xxxxx01111001  fmadd.w $w0, $w0, $w0
{            "fmax.d WREG , WREG , WREG",{0x1B00A07B,0xC0FF1F00}}, // xx011011xxxxxxxx101xxxxx01111011  fmax.d $w0, $w0, $w0
{            "fmax.w WREG , WREG , WREG",{0x1B00807B,0xC0FF1F00}}, // xx011011xxxxxxxx100xxxxx01111011  fmax.w $w0, $w0, $w0
{          "fmax_a.d WREG , WREG , WREG",{0x1B00E07B,0xC0FF1F00}}, // xx011011xxxxxxxx111xxxxx01111011  fmax_a.d $w0, $w0, $w0
{          "fmax_a.w WREG , WREG , WREG",{0x1B00C07B,0xC0FF1F00}}, // xx011011xxxxxxxx110xxxxx01111011  fmax_a.w $w0, $w0, $w0
{            "fmin.d WREG , WREG , WREG",{0x1B00207B,0xC0FF1F00}}, // xx011011xxxxxxxx001xxxxx01111011  fmin.d $w0, $w0, $w0
{            "fmin.w WREG , WREG , WREG",{0x1B00007B,0xC0FF1F00}}, // xx011011xxxxxxxx000xxxxx01111011  fmin.w $w0, $w0, $w0
{          "fmin_a.d WREG , WREG , WREG",{0x1B00607B,0xC0FF1F00}}, // xx011011xxxxxxxx011xxxxx01111011  fmin_a.d $w0, $w0, $w0
{          "fmin_a.w WREG , WREG , WREG",{0x1B00407B,0xC0FF1F00}}, // xx011011xxxxxxxx010xxxxx01111011  fmin_a.w $w0, $w0, $w0
{           "fmsub.d WREG , WREG , WREG",{0x1B006079,0xC0FF1F00}}, // xx011011xxxxxxxx011xxxxx01111001  fmsub.d $w0, $w0, $w0
{           "fmsub.w WREG , WREG , WREG",{0x1B004079,0xC0FF1F00}}, // xx011011xxxxxxxx010xxxxx01111001  fmsub.w $w0, $w0, $w0
{            "fmul.d WREG , WREG , WREG",{0x1B00A078,0xC0FF1F00}}, // xx011011xxxxxxxx101xxxxx01111000  fmul.d $w0, $w0, $w0
{            "fmul.w WREG , WREG , WREG",{0x1B008078,0xC0FF1F00}}, // xx011011xxxxxxxx100xxxxx01111000  fmul.w $w0, $w0, $w0
{                   "frcp.d WREG , WREG",{0x1E002B7B,0xC0FF0000}}, // xx011110xxxxxxxx0010101101111011  frcp.d $w0, $w0
{                   "frcp.w WREG , WREG",{0x1E002A7B,0xC0FF0000}}, // xx011110xxxxxxxx0010101001111011  frcp.w $w0, $w0
{                  "frint.d WREG , WREG",{0x1E002D7B,0xC0FF0000}}, // xx011110xxxxxxxx0010110101111011  frint.d $w0, $w0
{                  "frint.w WREG , WREG",{0x1E002C7B,0xC0FF0000}}, // xx011110xxxxxxxx0010110001111011  frint.w $w0, $w0
{                 "frsqrt.d WREG , WREG",{0x1E00297B,0xC0FF0000}}, // xx011110xxxxxxxx0010100101111011  frsqrt.d $w0, $w0
{                 "frsqrt.w WREG , WREG",{0x1E00287B,0xC0FF0000}}, // xx011110xxxxxxxx0010100001111011  frsqrt.w $w0, $w0
{            "fsaf.d WREG , WREG , WREG",{0x1A00207A,0xC0FF1F00}}, // xx011010xxxxxxxx001xxxxx01111010  fsaf.d $w0, $w0, $w0
{            "fsaf.w WREG , WREG , WREG",{0x1A00007A,0xC0FF1F00}}, // xx011010xxxxxxxx000xxxxx01111010  fsaf.w $w0, $w0, $w0
{            "fseq.d WREG , WREG , WREG",{0x1A00A07A,0xC0FF1F00}}, // xx011010xxxxxxxx101xxxxx01111010  fseq.d $w0, $w0, $w0
{            "fseq.w WREG , WREG , WREG",{0x1A00807A,0xC0FF1F00}}, // xx011010xxxxxxxx100xxxxx01111010  fseq.w $w0, $w0, $w0
{            "fsle.d WREG , WREG , WREG",{0x1A00A07B,0xC0FF1F00}}, // xx011010xxxxxxxx101xxxxx01111011  fsle.d $w0, $w0, $w0
{            "fsle.w WREG , WREG , WREG",{0x1A00807B,0xC0FF1F00}}, // xx011010xxxxxxxx100xxxxx01111011  fsle.w $w0, $w0, $w0
{            "fslt.d WREG , WREG , WREG",{0x1A00207B,0xC0FF1F00}}, // xx011010xxxxxxxx001xxxxx01111011  fslt.d $w0, $w0, $w0
{            "fslt.w WREG , WREG , WREG",{0x1A00007B,0xC0FF1F00}}, // xx011010xxxxxxxx000xxxxx01111011  fslt.w $w0, $w0, $w0
{            "fsne.d WREG , WREG , WREG",{0x1C00E07A,0xC0FF1F00}}, // xx011100xxxxxxxx111xxxxx01111010  fsne.d $w0, $w0, $w0
{            "fsne.w WREG , WREG , WREG",{0x1C00C07A,0xC0FF1F00}}, // xx011100xxxxxxxx110xxxxx01111010  fsne.w $w0, $w0, $w0
{            "fsor.d WREG , WREG , WREG",{0x1C00607A,0xC0FF1F00}}, // xx011100xxxxxxxx011xxxxx01111010  fsor.d $w0, $w0, $w0
{            "fsor.w WREG , WREG , WREG",{0x1C00407A,0xC0FF1F00}}, // xx011100xxxxxxxx010xxxxx01111010  fsor.w $w0, $w0, $w0
{                  "fsqrt.d WREG , WREG",{0x1E00277B,0xC0FF0000}}, // xx011110xxxxxxxx0010011101111011  fsqrt.d $w0, $w0
{                  "fsqrt.w WREG , WREG",{0x1E00267B,0xC0FF0000}}, // xx011110xxxxxxxx0010011001111011  fsqrt.w $w0, $w0
{            "fsub.d WREG , WREG , WREG",{0x1B006078,0xC0FF1F00}}, // xx011011xxxxxxxx011xxxxx01111000  fsub.d $w0, $w0, $w0
{            "fsub.w WREG , WREG , WREG",{0x1B004078,0xC0FF1F00}}, // xx011011xxxxxxxx010xxxxx01111000  fsub.w $w0, $w0, $w0
{           "fsueq.d WREG , WREG , WREG",{0x1A00E07A,0xC0FF1F00}}, // xx011010xxxxxxxx111xxxxx01111010  fsueq.d $w0, $w0, $w0
{           "fsueq.w WREG , WREG , WREG",{0x1A00C07A,0xC0FF1F00}}, // xx011010xxxxxxxx110xxxxx01111010  fsueq.w $w0, $w0, $w0
{           "fsule.d WREG , WREG , WREG",{0x1A00E07B,0xC0FF1F00}}, // xx011010xxxxxxxx111xxxxx01111011  fsule.d $w0, $w0, $w0
{           "fsule.w WREG , WREG , WREG",{0x1A00C07B,0xC0FF1F00}}, // xx011010xxxxxxxx110xxxxx01111011  fsule.w $w0, $w0, $w0
{           "fsult.d WREG , WREG , WREG",{0x1A00607B,0xC0FF1F00}}, // xx011010xxxxxxxx011xxxxx01111011  fsult.d $w0, $w0, $w0
{           "fsult.w WREG , WREG , WREG",{0x1A00407B,0xC0FF1F00}}, // xx011010xxxxxxxx010xxxxx01111011  fsult.w $w0, $w0, $w0
{            "fsun.d WREG , WREG , WREG",{0x1A00607A,0xC0FF1F00}}, // xx011010xxxxxxxx011xxxxx01111010  fsun.d $w0, $w0, $w0
{            "fsun.w WREG , WREG , WREG",{0x1A00407A,0xC0FF1F00}}, // xx011010xxxxxxxx010xxxxx01111010  fsun.w $w0, $w0, $w0
{           "fsune.d WREG , WREG , WREG",{0x1C00A07A,0xC0FF1F00}}, // xx011100xxxxxxxx101xxxxx01111010  fsune.d $w0, $w0, $w0
{           "fsune.w WREG , WREG , WREG",{0x1C00807A,0xC0FF1F00}}, // xx011100xxxxxxxx100xxxxx01111010  fsune.w $w0, $w0, $w0
{                "ftint_s.d WREG , WREG",{0x1E00397B,0xC0FF0000}}, // xx011110xxxxxxxx0011100101111011  ftint_s.d $w0, $w0
{                "ftint_s.w WREG , WREG",{0x1E00387B,0xC0FF0000}}, // xx011110xxxxxxxx0011100001111011  ftint_s.w $w0, $w0
{                "ftint_u.d WREG , WREG",{0x1E003B7B,0xC0FF0000}}, // xx011110xxxxxxxx0011101101111011  ftint_u.d $w0, $w0
{                "ftint_u.w WREG , WREG",{0x1E003A7B,0xC0FF0000}}, // xx011110xxxxxxxx0011101001111011  ftint_u.w $w0, $w0
{             "ftq.h WREG , WREG , WREG",{0x1B00807A,0xC0FF1F00}}, // xx011011xxxxxxxx100xxxxx01111010  ftq.h $w0, $w0, $w0
{             "ftq.w WREG , WREG , WREG",{0x1B00A07A,0xC0FF1F00}}, // xx011011xxxxxxxx101xxxxx01111010  ftq.w $w0, $w0, $w0
{               "ftrunc_s.d WREG , WREG",{0x1E00237B,0xC0FF0000}}, // xx011110xxxxxxxx0010001101111011  ftrunc_s.d $w0, $w0
{               "ftrunc_s.w WREG , WREG",{0x1E00227B,0xC0FF0000}}, // xx011110xxxxxxxx0010001001111011  ftrunc_s.w $w0, $w0
{               "ftrunc_u.d WREG , WREG",{0x1E00257B,0xC0FF0000}}, // xx011110xxxxxxxx0010010101111011  ftrunc_u.d $w0, $w0
{               "ftrunc_u.w WREG , WREG",{0x1E00247B,0xC0FF0000}}, // xx011110xxxxxxxx0010010001111011  ftrunc_u.w $w0, $w0
{          "hadd_s.d WREG , WREG , WREG",{0x1500607A,0xC0FF1F00}}, // xx010101xxxxxxxx011xxxxx01111010  hadd_s.d $w0, $w0, $w0
{          "hadd_s.h WREG , WREG , WREG",{0x1500207A,0xC0FF1F00}}, // xx010101xxxxxxxx001xxxxx01111010  hadd_s.h $w0, $w0, $w0
{          "hadd_s.w WREG , WREG , WREG",{0x1500407A,0xC0FF1F00}}, // xx010101xxxxxxxx010xxxxx01111010  hadd_s.w $w0, $w0, $w0
{          "hadd_u.d WREG , WREG , WREG",{0x1500E07A,0xC0FF1F00}}, // xx010101xxxxxxxx111xxxxx01111010  hadd_u.d $w0, $w0, $w0
{          "hadd_u.h WREG , WREG , WREG",{0x1500A07A,0xC0FF1F00}}, // xx010101xxxxxxxx101xxxxx01111010  hadd_u.h $w0, $w0, $w0
{          "hadd_u.w WREG , WREG , WREG",{0x1500C07A,0xC0FF1F00}}, // xx010101xxxxxxxx110xxxxx01111010  hadd_u.w $w0, $w0, $w0
{          "hsub_s.d WREG , WREG , WREG",{0x1500607B,0xC0FF1F00}}, // xx010101xxxxxxxx011xxxxx01111011  hsub_s.d $w0, $w0, $w0
{          "hsub_s.h WREG , WREG , WREG",{0x1500207B,0xC0FF1F00}}, // xx010101xxxxxxxx001xxxxx01111011  hsub_s.h $w0, $w0, $w0
{          "hsub_s.w WREG , WREG , WREG",{0x1500407B,0xC0FF1F00}}, // xx010101xxxxxxxx010xxxxx01111011  hsub_s.w $w0, $w0, $w0
{          "hsub_u.d WREG , WREG , WREG",{0x1500E07B,0xC0FF1F00}}, // xx010101xxxxxxxx111xxxxx01111011  hsub_u.d $w0, $w0, $w0
{          "hsub_u.h WREG , WREG , WREG",{0x1500A07B,0xC0FF1F00}}, // xx010101xxxxxxxx101xxxxx01111011  hsub_u.h $w0, $w0, $w0
{          "hsub_u.w WREG , WREG , WREG",{0x1500C07B,0xC0FF1F00}}, // xx010101xxxxxxxx110xxxxx01111011  hsub_u.w $w0, $w0, $w0
{           "ilvev.b WREG , WREG , WREG",{0x1400007B,0xC0FF1F00}}, // xx010100xxxxxxxx000xxxxx01111011  ilvev.b $w0, $w0, $w0
{           "ilvev.d WREG , WREG , WREG",{0x1400607B,0xC0FF1F00}}, // xx010100xxxxxxxx011xxxxx01111011  ilvev.d $w0, $w0, $w0
{           "ilvev.h WREG , WREG , WREG",{0x1400207B,0xC0FF1F00}}, // xx010100xxxxxxxx001xxxxx01111011  ilvev.h $w0, $w0, $w0
{           "ilvev.w WREG , WREG , WREG",{0x1400407B,0xC0FF1F00}}, // xx010100xxxxxxxx010xxxxx01111011  ilvev.w $w0, $w0, $w0
{            "ilvl.b WREG , WREG , WREG",{0x1400007A,0xC0FF1F00}}, // xx010100xxxxxxxx000xxxxx01111010  ilvl.b $w0, $w0, $w0
{            "ilvl.d WREG , WREG , WREG",{0x1400607A,0xC0FF1F00}}, // xx010100xxxxxxxx011xxxxx01111010  ilvl.d $w0, $w0, $w0
{            "ilvl.h WREG , WREG , WREG",{0x1400207A,0xC0FF1F00}}, // xx010100xxxxxxxx001xxxxx01111010  ilvl.h $w0, $w0, $w0
{            "ilvl.w WREG , WREG , WREG",{0x1400407A,0xC0FF1F00}}, // xx010100xxxxxxxx010xxxxx01111010  ilvl.w $w0, $w0, $w0
{           "ilvod.b WREG , WREG , WREG",{0x1400807B,0xC0FF1F00}}, // xx010100xxxxxxxx100xxxxx01111011  ilvod.b $w0, $w0, $w0
{           "ilvod.d WREG , WREG , WREG",{0x1400E07B,0xC0FF1F00}}, // xx010100xxxxxxxx111xxxxx01111011  ilvod.d $w0, $w0, $w0
{           "ilvod.h WREG , WREG , WREG",{0x1400A07B,0xC0FF1F00}}, // xx010100xxxxxxxx101xxxxx01111011  ilvod.h $w0, $w0, $w0
{           "ilvod.w WREG , WREG , WREG",{0x1400C07B,0xC0FF1F00}}, // xx010100xxxxxxxx110xxxxx01111011  ilvod.w $w0, $w0, $w0
{            "ilvr.b WREG , WREG , WREG",{0x1400807A,0xC0FF1F00}}, // xx010100xxxxxxxx100xxxxx01111010  ilvr.b $w0, $w0, $w0
{            "ilvr.d WREG , WREG , WREG",{0x1400E07A,0xC0FF1F00}}, // xx010100xxxxxxxx111xxxxx01111010  ilvr.d $w0, $w0, $w0
{            "ilvr.h WREG , WREG , WREG",{0x1400A07A,0xC0FF1F00}}, // xx010100xxxxxxxx101xxxxx01111010  ilvr.h $w0, $w0, $w0
{            "ilvr.w WREG , WREG , WREG",{0x1400C07A,0xC0FF1F00}}, // xx010100xxxxxxxx110xxxxx01111010  ilvr.w $w0, $w0, $w0
{        "ins GPREG , GPREG , NUM , NUM",{0x0400007C,0xC0FFFF03}}, // xx000100xxxxxxxxxxxxxxxx011111xx  ins $zero, $zero, 0, 1
{        "insert.b WREG [ NUM ] , GPREG",{0x19000079,0xC0FF0F00}}, // xx011001xxxxxxxx0000xxxx01111001  insert.b $w0[0], $zero
{        "insert.d WREG [ NUM ] , GPREG",{0x19003879,0xC0FF0100}}, // xx011001xxxxxxxx0011100x01111001  insert.d $w0[0], $zero
{        "insert.h WREG [ NUM ] , GPREG",{0x19002079,0xC0FF0700}}, // xx011001xxxxxxxx00100xxx01111001  insert.h $w0[0], $zero
{        "insert.w WREG [ NUM ] , GPREG",{0x19003079,0xC0FF0300}}, // xx011001xxxxxxxx001100xx01111001  insert.w $w0[0], $zero
{                   "insv GPREG , GPREG",{0x0C00007C,0x0000FF03}}, // 0000110000000000xxxxxxxx011111xx  insv $zero, $zero
{  "insve.b WREG [ NUM ] , WREG [ NUM ]",{0x19004079,0xC0FF0F00}}, // xx011001xxxxxxxx0100xxxx01111001  insve.b $w0[0], $w0[0]
{  "insve.d WREG [ NUM ] , WREG [ NUM ]",{0x19007879,0xC0FF0100}}, // xx011001xxxxxxxx0111100x01111001  insve.d $w0[0], $w0[0]
{  "insve.h WREG [ NUM ] , WREG [ NUM ]",{0x19006079,0xC0FF0700}}, // xx011001xxxxxxxx01100xxx01111001  insve.h $w0[0], $w0[0]
{  "insve.w WREG [ NUM ] , WREG [ NUM ]",{0x19007079,0xC0FF0300}}, // xx011001xxxxxxxx011100xx01111001  insve.w $w0[0], $w0[0]
{                                "j NUM",{0x00000008,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000010xx  j 0
{                              "jal NUM",{0x0000000C,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx000011xx  jal 0
{                   "jalr GPREG , GPREG",{0x09080000,0x00F8E003}}, // 00001001xxxxx000xxx00000000000xx  jalr $at, $zero
{                "jalr.hb GPREG , GPREG",{0x090C0000,0x00F8E003}}, // 00001001xxxxx100xxx00000000000xx  jalr.hb $at, $zero
{                          "jialc NUM ,",{0x000000F8,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx11111000  jialc 0,
{                            "jic NUM ,",{0x000000D8,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx000xxxxx11011000  jic 0,
{                             "jr GPREG",{0x08000000,0x0100E003}}, // 0000100x00000000xxx00000000000xx  jr $zero
{                          "jr.hb GPREG",{0x09040000,0x0000E003}}, // 0000100100000100xxx00000000000xx  jr.hb $zero
{                 "lb GPREG , ( GPREG )",{0x00000080,0x0000FF03}}, // 0000000000000000xxxxxxxx100000xx  lb $zero, ($zero)
{             "lb GPREG , NUM ( GPREG )",{0x00010080,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx100000xx  lb $zero, 0x100($zero)
{                "lbu GPREG , ( GPREG )",{0x00000090,0x0000FF03}}, // 0000000000000000xxxxxxxx100100xx  lbu $zero, ($zero)
{            "lbu GPREG , NUM ( GPREG )",{0x00010090,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx100100xx  lbu $zero, 0x100($zero)
{         "lbux GPREG , GPREG ( GPREG )",{0x8A01007C,0x00F8FF03}}, // 10001010xxxxx001xxxxxxxx011111xx  lbux $zero, $zero($zero)
{                "ld.b WREG , ( GPREG )",{0x20000078,0xC0FF0000}}, // xx100000xxxxxxxx0000000001111000  ld.b $w0, ($zero)
{            "ld.b WREG , NUM ( GPREG )",{0x2000007A,0xC0FFFF03}}, // xx100000xxxxxxxxxxxxxxxx011110xx  ld.b $w0, -0x200($zero)
{                "ld.d WREG , ( GPREG )",{0x23000078,0xC0FF0000}}, // xx100011xxxxxxxx0000000001111000  ld.d $w0, ($zero)
{            "ld.d WREG , NUM ( GPREG )",{0x2300007A,0xC0FFFF03}}, // xx100011xxxxxxxxxxxxxxxx011110xx  ld.d $w0, -0x1000($zero)
{                "ld.h WREG , ( GPREG )",{0x21000078,0xC0FF0000}}, // xx100001xxxxxxxx0000000001111000  ld.h $w0, ($zero)
{            "ld.h WREG , NUM ( GPREG )",{0x2100007A,0xC0FFFF03}}, // xx100001xxxxxxxxxxxxxxxx011110xx  ld.h $w0, -0x400($zero)
{                "ld.w WREG , ( GPREG )",{0x22000078,0xC0FF0000}}, // xx100010xxxxxxxx0000000001111000  ld.w $w0, ($zero)
{            "ld.w WREG , NUM ( GPREG )",{0x2200007A,0xC0FFFF03}}, // xx100010xxxxxxxxxxxxxxxx011110xx  ld.w $w0, -0x800($zero)
{                "ldc1 FREG , ( GPREG )",{0x000000D4,0x0000FF03}}, // 0000000000000000xxxxxxxx110101xx  ldc1 $f0, ($zero)
{            "ldc1 FREG , NUM ( GPREG )",{0x000100D4,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx110101xx  ldc1 $f0, 0x100($zero)
{                      "ldc2 CASH , ( )",{0x0000C049,0x00001F00}}, // 0000000000000000110xxxxx01001001  ldc2 $0, ()
{                  "ldc2 CASH , ( NUM )",{0x0001C049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx110xxxxx01001001  ldc2 $0, (0x100)
{                     "ldi.b WREG , NUM",{0x0700007B,0xC0FF1F00}}, // xx000111xxxxxxxx000xxxxx01111011  ldi.b $w0, 0
{                     "ldi.d WREG , NUM",{0x0700607B,0xC0FF1F00}}, // xx000111xxxxxxxx011xxxxx01111011  ldi.d $w0, 0
{                     "ldi.h WREG , NUM",{0x0700207B,0xC0FF1F00}}, // xx000111xxxxxxxx001xxxxx01111011  ldi.h $w0, 0
{                     "ldi.w WREG , NUM",{0x0700407B,0xC0FF1F00}}, // xx000111xxxxxxxx010xxxxx01111011  ldi.w $w0, 0
{                 "lh GPREG , ( GPREG )",{0x00000084,0x0000FF03}}, // 0000000000000000xxxxxxxx100001xx  lh $zero, ($zero)
{             "lh GPREG , NUM ( GPREG )",{0x00010084,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx100001xx  lh $zero, 0x100($zero)
{                "lhu GPREG , ( GPREG )",{0x00000094,0x0000FF03}}, // 0000000000000000xxxxxxxx100101xx  lhu $zero, ($zero)
{            "lhu GPREG , NUM ( GPREG )",{0x00010094,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx100101xx  lhu $zero, 0x100($zero)
{          "lhx GPREG , GPREG ( GPREG )",{0x0A01007C,0x00F8FF03}}, // 00001010xxxxx001xxxxxxxx011111xx  lhx $zero, $zero($zero)
{                 "ll GPREG , ( GPREG )",{0x3600007C,0x4000FF03}}, // 0x11011000000000xxxxxxxx011111xx  ll $zero, ($zero)
{             "ll GPREG , NUM ( GPREG )",{0x3601007C,0xC0FFFF03}}, // xx110110xxxxxxxxxxxxxxxx011111xx  ll $zero, 2($zero)
{                "lld GPREG , ( GPREG )",{0x3700007C,0x4000FF03}}, // 0x11011100000000xxxxxxxx011111xx  lld $zero, ($zero)
{            "lld GPREG , NUM ( GPREG )",{0x3701007C,0xC0FFFF03}}, // xx110111xxxxxxxxxxxxxxxx011111xx  lld $zero, 2($zero)
{      "lsa GPREG , GPREG , GPREG , NUM",{0x05000000,0xC0F8FF03}}, // xx000101xxxxx000xxxxxxxx000000xx  lsa $zero, $zero, $zero, 0
{                 "lw GPREG , ( GPREG )",{0x0000008C,0x0000FF03}}, // 0000000000000000xxxxxxxx100011xx  lw $zero, ($zero)
{             "lw GPREG , NUM ( GPREG )",{0x0001008C,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx100011xx  lw $zero, 0x100($zero)
{                "lwc1 FREG , ( GPREG )",{0x000000C4,0x0000FF03}}, // 0000000000000000xxxxxxxx110001xx  lwc1 $f0, ($zero)
{            "lwc1 FREG , NUM ( GPREG )",{0x000100C4,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx110001xx  lwc1 $f0, 0x100($zero)
{                      "lwc2 CASH , ( )",{0x00004049,0x00001F00}}, // 0000000000000000010xxxxx01001001  lwc2 $0, ()
{                  "lwc2 CASH , ( NUM )",{0x00014049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx010xxxxx01001001  lwc2 $0, (0x100)
{                     "lwpc GPREG , NUM",{0x000008EC,0xFFFFE703}}, // xxxxxxxxxxxxxxxxxxx01xxx111011xx  lwpc $zero, 0
{                    "lwupc GPREG , NUM",{0x000010EC,0xFFFFE703}}, // xxxxxxxxxxxxxxxxxxx10xxx111011xx  lwupc $zero, 0
{          "lwx GPREG , GPREG ( GPREG )",{0x0A00007C,0x00F8FF03}}, // 00001010xxxxx000xxxxxxxx011111xx  lwx $zero, $zero($zero)
{           "madd ACREG , GPREG , GPREG",{0x00000070,0x0018FF03}}, // 00000000000xx000xxxxxxxx011100xx  madd $ac0, $zero, $zero
{          "madd_q.h WREG , WREG , WREG",{0x1C004079,0xC0FF1F00}}, // xx011100xxxxxxxx010xxxxx01111001  madd_q.h $w0, $w0, $w0
{          "madd_q.w WREG , WREG , WREG",{0x1C006079,0xC0FF1F00}}, // xx011100xxxxxxxx011xxxxx01111001  madd_q.w $w0, $w0, $w0
{           "maddf.d FREG , FREG , FREG",{0x18002046,0xC0FF1F00}}, // xx011000xxxxxxxx001xxxxx01000110  maddf.d $f0, $f0, $f0
{           "maddf.s FREG , FREG , FREG",{0x18000046,0xC0FF1F00}}, // xx011000xxxxxxxx000xxxxx01000110  maddf.s $f0, $f0, $f0
{         "maddr_q.h WREG , WREG , WREG",{0x1C00407B,0xC0FF1F00}}, // xx011100xxxxxxxx010xxxxx01111011  maddr_q.h $w0, $w0, $w0
{         "maddr_q.w WREG , WREG , WREG",{0x1C00607B,0xC0FF1F00}}, // xx011100xxxxxxxx011xxxxx01111011  maddr_q.w $w0, $w0, $w0
{          "maddu ACREG , GPREG , GPREG",{0x01000070,0x0018FF03}}, // 00000001000xx000xxxxxxxx011100xx  maddu $ac0, $zero, $zero
{           "maddv.b WREG , WREG , WREG",{0x12008078,0xC0FF1F00}}, // xx010010xxxxxxxx100xxxxx01111000  maddv.b $w0, $w0, $w0
{           "maddv.d WREG , WREG , WREG",{0x1200E078,0xC0FF1F00}}, // xx010010xxxxxxxx111xxxxx01111000  maddv.d $w0, $w0, $w0
{           "maddv.h WREG , WREG , WREG",{0x1200A078,0xC0FF1F00}}, // xx010010xxxxxxxx101xxxxx01111000  maddv.h $w0, $w0, $w0
{           "maddv.w WREG , WREG , WREG",{0x1200C078,0xC0FF1F00}}, // xx010010xxxxxxxx110xxxxx01111000  maddv.w $w0, $w0, $w0
{    "maq_s.w.phl ACREG , GPREG , GPREG",{0x3005007C,0x0018FF03}}, // 00110000000xx101xxxxxxxx011111xx  maq_s.w.phl $ac0, $zero, $zero
{    "maq_s.w.phr ACREG , GPREG , GPREG",{0xB005007C,0x0018FF03}}, // 10110000000xx101xxxxxxxx011111xx  maq_s.w.phr $ac0, $zero, $zero
{   "maq_sa.w.phl ACREG , GPREG , GPREG",{0x3004007C,0x0018FF03}}, // 00110000000xx100xxxxxxxx011111xx  maq_sa.w.phl $ac0, $zero, $zero
{   "maq_sa.w.phr ACREG , GPREG , GPREG",{0xB004007C,0x0018FF03}}, // 10110000000xx100xxxxxxxx011111xx  maq_sa.w.phr $ac0, $zero, $zero
{             "max.d FREG , FREG , FREG",{0x1D002046,0xC0FF1F00}}, // xx011101xxxxxxxx001xxxxx01000110  max.d $f0, $f0, $f0
{             "max.s FREG , FREG , FREG",{0x1D000046,0xC0FF1F00}}, // xx011101xxxxxxxx000xxxxx01000110  max.s $f0, $f0, $f0
{           "max_a.b WREG , WREG , WREG",{0x0E00007B,0xC0FF1F00}}, // xx001110xxxxxxxx000xxxxx01111011  max_a.b $w0, $w0, $w0
{           "max_a.d WREG , WREG , WREG",{0x0E00607B,0xC0FF1F00}}, // xx001110xxxxxxxx011xxxxx01111011  max_a.d $w0, $w0, $w0
{           "max_a.h WREG , WREG , WREG",{0x0E00207B,0xC0FF1F00}}, // xx001110xxxxxxxx001xxxxx01111011  max_a.h $w0, $w0, $w0
{           "max_a.w WREG , WREG , WREG",{0x0E00407B,0xC0FF1F00}}, // xx001110xxxxxxxx010xxxxx01111011  max_a.w $w0, $w0, $w0
{           "max_s.b WREG , WREG , WREG",{0x0E000079,0xC0FF1F00}}, // xx001110xxxxxxxx000xxxxx01111001  max_s.b $w0, $w0, $w0
{           "max_s.d WREG , WREG , WREG",{0x0E006079,0xC0FF1F00}}, // xx001110xxxxxxxx011xxxxx01111001  max_s.d $w0, $w0, $w0
{           "max_s.h WREG , WREG , WREG",{0x0E002079,0xC0FF1F00}}, // xx001110xxxxxxxx001xxxxx01111001  max_s.h $w0, $w0, $w0
{           "max_s.w WREG , WREG , WREG",{0x0E004079,0xC0FF1F00}}, // xx001110xxxxxxxx010xxxxx01111001  max_s.w $w0, $w0, $w0
{           "max_u.b WREG , WREG , WREG",{0x0E008079,0xC0FF1F00}}, // xx001110xxxxxxxx100xxxxx01111001  max_u.b $w0, $w0, $w0
{           "max_u.d WREG , WREG , WREG",{0x0E00E079,0xC0FF1F00}}, // xx001110xxxxxxxx111xxxxx01111001  max_u.d $w0, $w0, $w0
{           "max_u.h WREG , WREG , WREG",{0x0E00A079,0xC0FF1F00}}, // xx001110xxxxxxxx101xxxxx01111001  max_u.h $w0, $w0, $w0
{           "max_u.w WREG , WREG , WREG",{0x0E00C079,0xC0FF1F00}}, // xx001110xxxxxxxx110xxxxx01111001  max_u.w $w0, $w0, $w0
{            "maxa.d FREG , FREG , FREG",{0x1F002046,0xC0FF1F00}}, // xx011111xxxxxxxx001xxxxx01000110  maxa.d $f0, $f0, $f0
{            "maxa.s FREG , FREG , FREG",{0x1F000046,0xC0FF1F00}}, // xx011111xxxxxxxx000xxxxx01000110  maxa.s $f0, $f0, $f0
{           "maxi_s.b WREG , WREG , NUM",{0x06000079,0xC0FF1F00}}, // xx000110xxxxxxxx000xxxxx01111001  maxi_s.b $w0, $w0, 0
{           "maxi_s.d WREG , WREG , NUM",{0x06006079,0xC0FF1F00}}, // xx000110xxxxxxxx011xxxxx01111001  maxi_s.d $w0, $w0, 0
{           "maxi_s.h WREG , WREG , NUM",{0x06002079,0xC0FF1F00}}, // xx000110xxxxxxxx001xxxxx01111001  maxi_s.h $w0, $w0, 0
{           "maxi_s.w WREG , WREG , NUM",{0x06004079,0xC0FF1F00}}, // xx000110xxxxxxxx010xxxxx01111001  maxi_s.w $w0, $w0, 0
{           "maxi_u.b WREG , WREG , NUM",{0x06008079,0xC0FF1F00}}, // xx000110xxxxxxxx100xxxxx01111001  maxi_u.b $w0, $w0, 0
{           "maxi_u.d WREG , WREG , NUM",{0x0600E079,0xC0FF1F00}}, // xx000110xxxxxxxx111xxxxx01111001  maxi_u.d $w0, $w0, 0
{           "maxi_u.h WREG , WREG , NUM",{0x0600A079,0xC0FF1F00}}, // xx000110xxxxxxxx101xxxxx01111001  maxi_u.h $w0, $w0, 0
{           "maxi_u.w WREG , WREG , NUM",{0x0600C079,0xC0FF1F00}}, // xx000110xxxxxxxx110xxxxx01111001  maxi_u.w $w0, $w0, 0
{             "mfc0 GPREG , GPREG , NUM",{0x00000040,0x07F81F00}}, // 00000xxxxxxxx000000xxxxx01000000  mfc0 $zero, $zero, 0
{                    "mfc1 GPREG , FREG",{0x00000044,0x00F81F00}}, // 00000000xxxxx000000xxxxx01000100  mfc1 $zero, $f0
{             "mfc2 GPREG , GPREG , NUM",{0x00000048,0x07F81F00}}, // 00000xxxxxxxx000000xxxxx01001000  mfc2 $zero, $zero, 0
{                   "mfhc1 GPREG , FREG",{0x00006044,0x00F01F00}}, // 00000000xxxx0000011xxxxx01000100  mfhc1 $zero, $f0
{                   "mfhi GPREG , ACREG",{0x10000000,0x00F86000}}, // 00010000xxxxx0000xx0000000000000  mfhi $zero, $ac0
{                   "mflo GPREG , ACREG",{0x12000000,0x00F86000}}, // 00010010xxxxx0000xx0000000000000  mflo $zero, $ac0
{             "min.d FREG , FREG , FREG",{0x1C002046,0xC0FF1F00}}, // xx011100xxxxxxxx001xxxxx01000110  min.d $f0, $f0, $f0
{             "min.s FREG , FREG , FREG",{0x1C000046,0xC0FF1F00}}, // xx011100xxxxxxxx000xxxxx01000110  min.s $f0, $f0, $f0
{           "min_a.b WREG , WREG , WREG",{0x0E00807B,0xC0FF1F00}}, // xx001110xxxxxxxx100xxxxx01111011  min_a.b $w0, $w0, $w0
{           "min_a.d WREG , WREG , WREG",{0x0E00E07B,0xC0FF1F00}}, // xx001110xxxxxxxx111xxxxx01111011  min_a.d $w0, $w0, $w0
{           "min_a.h WREG , WREG , WREG",{0x0E00A07B,0xC0FF1F00}}, // xx001110xxxxxxxx101xxxxx01111011  min_a.h $w0, $w0, $w0
{           "min_a.w WREG , WREG , WREG",{0x0E00C07B,0xC0FF1F00}}, // xx001110xxxxxxxx110xxxxx01111011  min_a.w $w0, $w0, $w0
{           "min_s.b WREG , WREG , WREG",{0x0E00007A,0xC0FF1F00}}, // xx001110xxxxxxxx000xxxxx01111010  min_s.b $w0, $w0, $w0
{           "min_s.d WREG , WREG , WREG",{0x0E00607A,0xC0FF1F00}}, // xx001110xxxxxxxx011xxxxx01111010  min_s.d $w0, $w0, $w0
{           "min_s.h WREG , WREG , WREG",{0x0E00207A,0xC0FF1F00}}, // xx001110xxxxxxxx001xxxxx01111010  min_s.h $w0, $w0, $w0
{           "min_s.w WREG , WREG , WREG",{0x0E00407A,0xC0FF1F00}}, // xx001110xxxxxxxx010xxxxx01111010  min_s.w $w0, $w0, $w0
{           "min_u.b WREG , WREG , WREG",{0x0E00807A,0xC0FF1F00}}, // xx001110xxxxxxxx100xxxxx01111010  min_u.b $w0, $w0, $w0
{           "min_u.d WREG , WREG , WREG",{0x0E00E07A,0xC0FF1F00}}, // xx001110xxxxxxxx111xxxxx01111010  min_u.d $w0, $w0, $w0
{           "min_u.h WREG , WREG , WREG",{0x0E00A07A,0xC0FF1F00}}, // xx001110xxxxxxxx101xxxxx01111010  min_u.h $w0, $w0, $w0
{           "min_u.w WREG , WREG , WREG",{0x0E00C07A,0xC0FF1F00}}, // xx001110xxxxxxxx110xxxxx01111010  min_u.w $w0, $w0, $w0
{            "mina.d FREG , FREG , FREG",{0x1E002046,0xC0FF1F00}}, // xx011110xxxxxxxx001xxxxx01000110  mina.d $f0, $f0, $f0
{            "mina.s FREG , FREG , FREG",{0x1E000046,0xC0FF1F00}}, // xx011110xxxxxxxx000xxxxx01000110  mina.s $f0, $f0, $f0
{           "mini_s.b WREG , WREG , NUM",{0x0600007A,0xC0FF1F00}}, // xx000110xxxxxxxx000xxxxx01111010  mini_s.b $w0, $w0, 0
{           "mini_s.d WREG , WREG , NUM",{0x0600607A,0xC0FF1F00}}, // xx000110xxxxxxxx011xxxxx01111010  mini_s.d $w0, $w0, 0
{           "mini_s.h WREG , WREG , NUM",{0x0600207A,0xC0FF1F00}}, // xx000110xxxxxxxx001xxxxx01111010  mini_s.h $w0, $w0, 0
{           "mini_s.w WREG , WREG , NUM",{0x0600407A,0xC0FF1F00}}, // xx000110xxxxxxxx010xxxxx01111010  mini_s.w $w0, $w0, 0
{           "mini_u.b WREG , WREG , NUM",{0x0600807A,0xC0FF1F00}}, // xx000110xxxxxxxx100xxxxx01111010  mini_u.b $w0, $w0, 0
{           "mini_u.d WREG , WREG , NUM",{0x0600E07A,0xC0FF1F00}}, // xx000110xxxxxxxx111xxxxx01111010  mini_u.d $w0, $w0, 0
{           "mini_u.h WREG , WREG , NUM",{0x0600A07A,0xC0FF1F00}}, // xx000110xxxxxxxx101xxxxx01111010  mini_u.h $w0, $w0, 0
{           "mini_u.w WREG , WREG , NUM",{0x0600C07A,0xC0FF1F00}}, // xx000110xxxxxxxx110xxxxx01111010  mini_u.w $w0, $w0, 0
{            "mod GPREG , GPREG , GPREG",{0xDA000000,0x00F8FF03}}, // 11011010xxxxx000xxxxxxxx000000xx  mod $zero, $zero, $zero
{           "mod_s.b WREG , WREG , WREG",{0x1200007B,0xC0FF1F00}}, // xx010010xxxxxxxx000xxxxx01111011  mod_s.b $w0, $w0, $w0
{           "mod_s.d WREG , WREG , WREG",{0x1200607B,0xC0FF1F00}}, // xx010010xxxxxxxx011xxxxx01111011  mod_s.d $w0, $w0, $w0
{           "mod_s.h WREG , WREG , WREG",{0x1200207B,0xC0FF1F00}}, // xx010010xxxxxxxx001xxxxx01111011  mod_s.h $w0, $w0, $w0
{           "mod_s.w WREG , WREG , WREG",{0x1200407B,0xC0FF1F00}}, // xx010010xxxxxxxx010xxxxx01111011  mod_s.w $w0, $w0, $w0
{           "mod_u.b WREG , WREG , WREG",{0x1200807B,0xC0FF1F00}}, // xx010010xxxxxxxx100xxxxx01111011  mod_u.b $w0, $w0, $w0
{           "mod_u.d WREG , WREG , WREG",{0x1200E07B,0xC0FF1F00}}, // xx010010xxxxxxxx111xxxxx01111011  mod_u.d $w0, $w0, $w0
{           "mod_u.h WREG , WREG , WREG",{0x1200A07B,0xC0FF1F00}}, // xx010010xxxxxxxx101xxxxx01111011  mod_u.h $w0, $w0, $w0
{           "mod_u.w WREG , WREG , WREG",{0x1200C07B,0xC0FF1F00}}, // xx010010xxxxxxxx110xxxxx01111011  mod_u.w $w0, $w0, $w0
{         "modsub GPREG , GPREG , GPREG",{0x9004007C,0x00F8FF03}}, // 10010000xxxxx100xxxxxxxx011111xx  modsub $zero, $zero, $zero
{           "modu GPREG , GPREG , GPREG",{0xDB000000,0x00F8FF03}}, // 11011011xxxxx000xxxxxxxx000000xx  modu $zero, $zero, $zero
{                    "mov.d FREG , FREG",{0x06002046,0x80F70000}}, // x0000110xxxx0xxx0010000001000110  mov.d $f0, $f0
{                    "mov.s FREG , FREG",{0x06000046,0xC0FF0000}}, // xx000110xxxxxxxx0000000001000110  mov.s $f0, $f0
{                   "move GPREG , GPREG",{0x21000000,0x04F8E003}}, // 00100x01xxxxx000xxx00000000000xx  move $zero, $zero
{                   "move.v WREG , WREG",{0x1900BE78,0xC0FF0000}}, // xx011001xxxxxxxx1011111001111000  move.v $w0, $w0
{           "msub ACREG , GPREG , GPREG",{0x04000070,0x0018FF03}}, // 00000100000xx000xxxxxxxx011100xx  msub $ac0, $zero, $zero
{          "msub_q.h WREG , WREG , WREG",{0x1C008079,0xC0FF1F00}}, // xx011100xxxxxxxx100xxxxx01111001  msub_q.h $w0, $w0, $w0
{          "msub_q.w WREG , WREG , WREG",{0x1C00A079,0xC0FF1F00}}, // xx011100xxxxxxxx101xxxxx01111001  msub_q.w $w0, $w0, $w0
{           "msubf.d FREG , FREG , FREG",{0x19002046,0xC0FF1F00}}, // xx011001xxxxxxxx001xxxxx01000110  msubf.d $f0, $f0, $f0
{           "msubf.s FREG , FREG , FREG",{0x19000046,0xC0FF1F00}}, // xx011001xxxxxxxx000xxxxx01000110  msubf.s $f0, $f0, $f0
{         "msubr_q.h WREG , WREG , WREG",{0x1C00807B,0xC0FF1F00}}, // xx011100xxxxxxxx100xxxxx01111011  msubr_q.h $w0, $w0, $w0
{         "msubr_q.w WREG , WREG , WREG",{0x1C00A07B,0xC0FF1F00}}, // xx011100xxxxxxxx101xxxxx01111011  msubr_q.w $w0, $w0, $w0
{          "msubu ACREG , GPREG , GPREG",{0x05000070,0x0018FF03}}, // 00000101000xx000xxxxxxxx011100xx  msubu $ac0, $zero, $zero
{           "msubv.b WREG , WREG , WREG",{0x12000079,0xC0FF1F00}}, // xx010010xxxxxxxx000xxxxx01111001  msubv.b $w0, $w0, $w0
{           "msubv.d WREG , WREG , WREG",{0x12006079,0xC0FF1F00}}, // xx010010xxxxxxxx011xxxxx01111001  msubv.d $w0, $w0, $w0
{           "msubv.h WREG , WREG , WREG",{0x12002079,0xC0FF1F00}}, // xx010010xxxxxxxx001xxxxx01111001  msubv.h $w0, $w0, $w0
{           "msubv.w WREG , WREG , WREG",{0x12004079,0xC0FF1F00}}, // xx010010xxxxxxxx010xxxxx01111001  msubv.w $w0, $w0, $w0
{             "mtc0 GPREG , GPREG , NUM",{0x00008040,0x07F81F00}}, // 00000xxxxxxxx000100xxxxx01000000  mtc0 $zero, $zero, 0
{                    "mtc1 GPREG , FREG",{0x00008044,0x00F81F00}}, // 00000000xxxxx000100xxxxx01000100  mtc1 $zero, $f0
{             "mtc2 GPREG , GPREG , NUM",{0x00008048,0x07F81F00}}, // 00000xxxxxxxx000100xxxxx01001000  mtc2 $zero, $zero, 0
{                   "mthc1 GPREG , FREG",{0x0000E044,0x00F01F00}}, // 00000000xxxx0000111xxxxx01000100  mthc1 $zero, $f0
{                   "mthi GPREG , ACREG",{0x11000000,0x0018E003}}, // 00010001000xx000xxx00000000000xx  mthi $zero, $ac0
{                 "mthlip GPREG , ACREG",{0xF807007C,0x0018E003}}, // 11111000000xx111xxx00000011111xx  mthlip $zero, $ac0
{                   "mtlo GPREG , ACREG",{0x13000000,0x0018E003}}, // 00010011000xx000xxx00000000000xx  mtlo $zero, $ac0
{            "muh GPREG , GPREG , GPREG",{0xD8000000,0x00F8FF03}}, // 11011000xxxxx000xxxxxxxx000000xx  muh $zero, $zero, $zero
{           "muhu GPREG , GPREG , GPREG",{0xD9000000,0x00F8FF03}}, // 11011001xxxxx000xxxxxxxx000000xx  muhu $zero, $zero, $zero
{            "mul GPREG , GPREG , GPREG",{0x98000000,0x00F8FF03}}, // 10011000xxxxx000xxxxxxxx000000xx  mul $zero, $zero, $zero
{             "mul.d FREG , FREG , FREG",{0x02002046,0x80F71E00}}, // x0000010xxxx0xxx001xxxx001000110  mul.d $f0, $f0, $f0
{         "mul.ph GPREG , GPREG , GPREG",{0x1803007C,0x00F8FF03}}, // 00011000xxxxx011xxxxxxxx011111xx  mul.ph $zero, $zero, $zero
{             "mul.s FREG , FREG , FREG",{0x02000046,0xC0FF1F00}}, // xx000010xxxxxxxx000xxxxx01000110  mul.s $f0, $f0, $f0
{           "mul_q.h WREG , WREG , WREG",{0x1C000079,0xC0FF1F00}}, // xx011100xxxxxxxx000xxxxx01111001  mul_q.h $w0, $w0, $w0
{           "mul_q.w WREG , WREG , WREG",{0x1C002079,0xC0FF1F00}}, // xx011100xxxxxxxx001xxxxx01111001  mul_q.w $w0, $w0, $w0
{       "mul_s.ph GPREG , GPREG , GPREG",{0x9803007C,0x00F8FF03}}, // 10011000xxxxx011xxxxxxxx011111xx  mul_s.ph $zero, $zero, $zero
{  "muleq_s.w.phl GPREG , GPREG , GPREG",{0x1007007C,0x00F8FF03}}, // 00010000xxxxx111xxxxxxxx011111xx  muleq_s.w.phl $zero, $zero, $zero
{  "muleq_s.w.phr GPREG , GPREG , GPREG",{0x5007007C,0x00F8FF03}}, // 01010000xxxxx111xxxxxxxx011111xx  muleq_s.w.phr $zero, $zero, $zero
{ "muleu_s.ph.qbl GPREG , GPREG , GPREG",{0x9001007C,0x00F8FF03}}, // 10010000xxxxx001xxxxxxxx011111xx  muleu_s.ph.qbl $zero, $zero, $zero
{ "muleu_s.ph.qbr GPREG , GPREG , GPREG",{0xD001007C,0x00F8FF03}}, // 11010000xxxxx001xxxxxxxx011111xx  muleu_s.ph.qbr $zero, $zero, $zero
{     "mulq_rs.ph GPREG , GPREG , GPREG",{0xD007007C,0x00F8FF03}}, // 11010000xxxxx111xxxxxxxx011111xx  mulq_rs.ph $zero, $zero, $zero
{      "mulq_rs.w GPREG , GPREG , GPREG",{0xD805007C,0x00F8FF03}}, // 11011000xxxxx101xxxxxxxx011111xx  mulq_rs.w $zero, $zero, $zero
{      "mulq_s.ph GPREG , GPREG , GPREG",{0x9007007C,0x00F8FF03}}, // 10010000xxxxx111xxxxxxxx011111xx  mulq_s.ph $zero, $zero, $zero
{       "mulq_s.w GPREG , GPREG , GPREG",{0x9805007C,0x00F8FF03}}, // 10011000xxxxx101xxxxxxxx011111xx  mulq_s.w $zero, $zero, $zero
{          "mulr_q.h WREG , WREG , WREG",{0x1C00007B,0xC0FF1F00}}, // xx011100xxxxxxxx000xxxxx01111011  mulr_q.h $w0, $w0, $w0
{          "mulr_q.w WREG , WREG , WREG",{0x1C00207B,0xC0FF1F00}}, // xx011100xxxxxxxx001xxxxx01111011  mulr_q.w $w0, $w0, $w0
{     "mulsa.w.ph ACREG , GPREG , GPREG",{0xB000007C,0x0018FF03}}, // 10110000000xx000xxxxxxxx011111xx  mulsa.w.ph $ac0, $zero, $zero
{  "mulsaq_s.w.ph ACREG , GPREG , GPREG",{0xB001007C,0x0018FF03}}, // 10110000000xx001xxxxxxxx011111xx  mulsaq_s.w.ph $ac0, $zero, $zero
{           "mult ACREG , GPREG , GPREG",{0x18000000,0x0018FF03}}, // 00011000000xx000xxxxxxxx000000xx  mult $ac0, $zero, $zero
{          "multu ACREG , GPREG , GPREG",{0x19000000,0x0018FF03}}, // 00011001000xx000xxxxxxxx000000xx  multu $ac0, $zero, $zero
{           "mulu GPREG , GPREG , GPREG",{0x99000000,0x00F8FF03}}, // 10011001xxxxx000xxxxxxxx000000xx  mulu $zero, $zero, $zero
{            "mulv.b WREG , WREG , WREG",{0x12000078,0xC0FF1F00}}, // xx010010xxxxxxxx000xxxxx01111000  mulv.b $w0, $w0, $w0
{            "mulv.d WREG , WREG , WREG",{0x12006078,0xC0FF1F00}}, // xx010010xxxxxxxx011xxxxx01111000  mulv.d $w0, $w0, $w0
{            "mulv.h WREG , WREG , WREG",{0x12002078,0xC0FF1F00}}, // xx010010xxxxxxxx001xxxxx01111000  mulv.h $w0, $w0, $w0
{            "mulv.w WREG , WREG , WREG",{0x12004078,0xC0FF1F00}}, // xx010010xxxxxxxx010xxxxx01111000  mulv.w $w0, $w0, $w0
{                    "neg GPREG , GPREG",{0x22000000,0x00F81F00}}, // 00100010xxxxx000000xxxxx00000000  neg $zero, $zero
{                    "neg.d FREG , FREG",{0x07002046,0x80F70000}}, // x0000111xxxx0xxx0010000001000110  neg.d $f0, $f0
{                    "neg.s FREG , FREG",{0x07000046,0xC0FF0000}}, // xx000111xxxxxxxx0000000001000110  neg.s $f0, $f0
{                   "negu GPREG , GPREG",{0x23000000,0x00F81F00}}, // 00100011xxxxx000000xxxxx00000000  negu $zero, $zero
{                   "nloc.b WREG , WREG",{0x1E00087B,0xC0FF0000}}, // xx011110xxxxxxxx0000100001111011  nloc.b $w0, $w0
{                   "nloc.d WREG , WREG",{0x1E000B7B,0xC0FF0000}}, // xx011110xxxxxxxx0000101101111011  nloc.d $w0, $w0
{                   "nloc.h WREG , WREG",{0x1E00097B,0xC0FF0000}}, // xx011110xxxxxxxx0000100101111011  nloc.h $w0, $w0
{                   "nloc.w WREG , WREG",{0x1E000A7B,0xC0FF0000}}, // xx011110xxxxxxxx0000101001111011  nloc.w $w0, $w0
{                   "nlzc.b WREG , WREG",{0x1E000C7B,0xC0FF0000}}, // xx011110xxxxxxxx0000110001111011  nlzc.b $w0, $w0
{                   "nlzc.d WREG , WREG",{0x1E000F7B,0xC0FF0000}}, // xx011110xxxxxxxx0000111101111011  nlzc.d $w0, $w0
{                   "nlzc.h WREG , WREG",{0x1E000D7B,0xC0FF0000}}, // xx011110xxxxxxxx0000110101111011  nlzc.h $w0, $w0
{                   "nlzc.w WREG , WREG",{0x1E000E7B,0xC0FF0000}}, // xx011110xxxxxxxx0000111001111011  nlzc.w $w0, $w0
{                                  "nop",{0x00000000,0x00000000}}, // 00000000000000000000000000000000  nop
{            "nor GPREG , GPREG , GPREG",{0x27000100,0x00F8FF03}}, // 00100111xxxxx000xxxxxxxx000000xx  nor $zero, $zero, $at
{             "nor.v WREG , WREG , WREG",{0x1E004078,0xC0FF1F00}}, // xx011110xxxxxxxx010xxxxx01111000  nor.v $w0, $w0, $w0
{             "nori.b WREG , WREG , NUM",{0x0000007A,0xC0FFFF00}}, // xx000000xxxxxxxxxxxxxxxx01111010  nori.b $w0, $w0, 0
{                    "not GPREG , GPREG",{0x27000000,0x00F8E003}}, // 00100111xxxxx000xxx00000000000xx  not $zero, $zero
{             "or GPREG , GPREG , GPREG",{0x25000100,0x00F8FF03}}, // 00100101xxxxx000xxxxxxxx000000xx  or $zero, $zero, $at
{              "or.v WREG , WREG , WREG",{0x1E002078,0xC0FF1F00}}, // xx011110xxxxxxxx001xxxxx01111000  or.v $w0, $w0, $w0
{              "ori GPREG , GPREG , NUM",{0x00000034,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001101xx  ori $zero, $zero, 0
{              "ori.b WREG , WREG , NUM",{0x00000079,0xC0FFFF00}}, // xx000000xxxxxxxxxxxxxxxx01111001  ori.b $w0, $w0, 0
{      "packrl.ph GPREG , GPREG , GPREG",{0x9103007C,0x00F8FF03}}, // 10010001xxxxx011xxxxxxxx011111xx  packrl.ph $zero, $zero, $zero
{                                "pause",{0x40010000,0x00000000}}, // 01000000000000010000000000000000  pause
{           "pckev.b WREG , WREG , WREG",{0x14000079,0xC0FF1F00}}, // xx010100xxxxxxxx000xxxxx01111001  pckev.b $w0, $w0, $w0
{           "pckev.d WREG , WREG , WREG",{0x14006079,0xC0FF1F00}}, // xx010100xxxxxxxx011xxxxx01111001  pckev.d $w0, $w0, $w0
{           "pckev.h WREG , WREG , WREG",{0x14002079,0xC0FF1F00}}, // xx010100xxxxxxxx001xxxxx01111001  pckev.h $w0, $w0, $w0
{           "pckev.w WREG , WREG , WREG",{0x14004079,0xC0FF1F00}}, // xx010100xxxxxxxx010xxxxx01111001  pckev.w $w0, $w0, $w0
{           "pckod.b WREG , WREG , WREG",{0x14008079,0xC0FF1F00}}, // xx010100xxxxxxxx100xxxxx01111001  pckod.b $w0, $w0, $w0
{           "pckod.d WREG , WREG , WREG",{0x1400E079,0xC0FF1F00}}, // xx010100xxxxxxxx111xxxxx01111001  pckod.d $w0, $w0, $w0
{           "pckod.h WREG , WREG , WREG",{0x1400A079,0xC0FF1F00}}, // xx010100xxxxxxxx101xxxxx01111001  pckod.h $w0, $w0, $w0
{           "pckod.w WREG , WREG , WREG",{0x1400C079,0xC0FF1F00}}, // xx010100xxxxxxxx110xxxxx01111001  pckod.w $w0, $w0, $w0
{                   "pcnt.b WREG , WREG",{0x1E00047B,0xC0FF0000}}, // xx011110xxxxxxxx0000010001111011  pcnt.b $w0, $w0
{                   "pcnt.d WREG , WREG",{0x1E00077B,0xC0FF0000}}, // xx011110xxxxxxxx0000011101111011  pcnt.d $w0, $w0
{                   "pcnt.h WREG , WREG",{0x1E00057B,0xC0FF0000}}, // xx011110xxxxxxxx0000010101111011  pcnt.h $w0, $w0
{                   "pcnt.w WREG , WREG",{0x1E00067B,0xC0FF0000}}, // xx011110xxxxxxxx0000011001111011  pcnt.w $w0, $w0
{        "pick.ph GPREG , GPREG , GPREG",{0xD102007C,0x00F8FF03}}, // 11010001xxxxx010xxxxxxxx011111xx  pick.ph $zero, $zero, $zero
{        "pick.qb GPREG , GPREG , GPREG",{0xD100007C,0x00F8FF03}}, // 11010001xxxxx000xxxxxxxx011111xx  pick.qb $zero, $zero, $zero
{           "preceq.w.phl GPREG , GPREG",{0x1203007C,0x00F81F00}}, // 00010010xxxxx011000xxxxx01111100  preceq.w.phl $zero, $zero
{           "preceq.w.phr GPREG , GPREG",{0x5203007C,0x00F81F00}}, // 01010010xxxxx011000xxxxx01111100  preceq.w.phr $zero, $zero
{         "precequ.ph.qbl GPREG , GPREG",{0x1201007C,0x00F81F00}}, // 00010010xxxxx001000xxxxx01111100  precequ.ph.qbl $zero, $zero
{        "precequ.ph.qbla GPREG , GPREG",{0x9201007C,0x00F81F00}}, // 10010010xxxxx001000xxxxx01111100  precequ.ph.qbla $zero, $zero
{         "precequ.ph.qbr GPREG , GPREG",{0x5201007C,0x00F81F00}}, // 01010010xxxxx001000xxxxx01111100  precequ.ph.qbr $zero, $zero
{        "precequ.ph.qbra GPREG , GPREG",{0xD201007C,0x00F81F00}}, // 11010010xxxxx001000xxxxx01111100  precequ.ph.qbra $zero, $zero
{          "preceu.ph.qbl GPREG , GPREG",{0x1207007C,0x00F81F00}}, // 00010010xxxxx111000xxxxx01111100  preceu.ph.qbl $zero, $zero
{         "preceu.ph.qbla GPREG , GPREG",{0x9207007C,0x00F81F00}}, // 10010010xxxxx111000xxxxx01111100  preceu.ph.qbla $zero, $zero
{          "preceu.ph.qbr GPREG , GPREG",{0x5207007C,0x00F81F00}}, // 01010010xxxxx111000xxxxx01111100  preceu.ph.qbr $zero, $zero
{         "preceu.ph.qbra GPREG , GPREG",{0xD207007C,0x00F81F00}}, // 11010010xxxxx111000xxxxx01111100  preceu.ph.qbra $zero, $zero
{    "precr.qb.ph GPREG , GPREG , GPREG",{0x5103007C,0x00F8FF03}}, // 01010001xxxxx011xxxxxxxx011111xx  precr.qb.ph $zero, $zero, $zero
{   "precr_sra.ph.w GPREG , GPREG , NUM",{0x9107007C,0x00F8FF03}}, // 10010001xxxxx111xxxxxxxx011111xx  precr_sra.ph.w $zero, $zero, 0
{ "precr_sra_r.ph.w GPREG , GPREG , NUM",{0xD107007C,0x00F8FF03}}, // 11010001xxxxx111xxxxxxxx011111xx  precr_sra_r.ph.w $zero, $zero, 0
{    "precrq.ph.w GPREG , GPREG , GPREG",{0x1105007C,0x00F8FF03}}, // 00010001xxxxx101xxxxxxxx011111xx  precrq.ph.w $zero, $zero, $zero
{   "precrq.qb.ph GPREG , GPREG , GPREG",{0x1103007C,0x00F8FF03}}, // 00010001xxxxx011xxxxxxxx011111xx  precrq.qb.ph $zero, $zero, $zero
{ "precrq_rs.ph.w GPREG , GPREG , GPREG",{0x5105007C,0x00F8FF03}}, // 01010001xxxxx101xxxxxxxx011111xx  precrq_rs.ph.w $zero, $zero, $zero
{"precrqu_s.qb.ph GPREG , GPREG , GPREG",{0xD103007C,0x00F8FF03}}, // 11010001xxxxx011xxxxxxxx011111xx  precrqu_s.qb.ph $zero, $zero, $zero
{                           "pref , ( )",{0x3500007C,0x00000000}}, // 00110101000000000000000001111100  pref , ()
{                       "pref , ( NUM )",{0x3500007E,0x80FFE003}}, // x0110101xxxxxxxxxxx00000011111xx  pref , (0x100000)
{                       "pref , NUM ( )",{0x3500017C,0x00001F00}}, // 0011010100000000000xxxxx01111100  pref , 1()
{                   "pref , NUM ( NUM )",{0x3500017E,0x80FFFF03}}, // x0110101xxxxxxxxxxxxxxxx011111xx  pref , 1(0x100000)
{          "prepend GPREG , GPREG , NUM",{0x7100007C,0x00F8FF03}}, // 01110001xxxxx000xxxxxxxx011111xx  prepend $zero, $zero, 0
{             "raddu.w.qb GPREG , GPREG",{0x1005007C,0x00F8E003}}, // 00010000xxxxx101xxx00000011111xx  raddu.w.qb $zero, $zero
{                    "rddsp GPREG , NUM",{0xB804007C,0x00F8FF03}}, // 10111000xxxxx100xxxxxxxx011111xx  rddsp $zero, 0
{                   "rdhwr GPREG , CASH",{0x3BE8007C,0x00001F00}}, // 0011101111101000000xxxxx01111100  rdhwr $zero, $29
{                  "repl.ph GPREG , NUM",{0x9202007C,0x00F8FF03}}, // 10010010xxxxx010xxxxxxxx011111xx  repl.ph $zero, 0
{                  "repl.qb GPREG , NUM",{0x9200007C,0x00F8FF03}}, // 10010010xxxxx000xxxxxxxx011111xx  repl.qb $zero, 0
{               "replv.ph GPREG , GPREG",{0xD202007C,0x00F81F00}}, // 11010010xxxxx010000xxxxx01111100  replv.ph $zero, $zero
{               "replv.qb GPREG , GPREG",{0xD200007C,0x00F81F00}}, // 11010010xxxxx000000xxxxx01111100  replv.qb $zero, $zero
{                   "rint.d FREG , FREG",{0x1A002046,0xC0FF0000}}, // xx011010xxxxxxxx0010000001000110  rint.d $f0, $f0
{                   "rint.s FREG , FREG",{0x1A000046,0xC0FF0000}}, // xx011010xxxxxxxx0000000001000110  rint.s $f0, $f0
{             "rotr GPREG , GPREG , NUM",{0x02002000,0xC0FF1F00}}, // xx000010xxxxxxxx001xxxxx00000000  rotr $zero, $zero, 0
{          "rotrv GPREG , GPREG , GPREG",{0x46000000,0x00F8FF03}}, // 01000110xxxxx000xxxxxxxx000000xx  rotrv $zero, $zero, $zero
{                "round.w.d FREG , FREG",{0x0C002046,0xC0F70000}}, // xx001100xxxx0xxx0010000001000110  round.w.d $f0, $f0
{                "round.w.s FREG , FREG",{0x0C000046,0xC0FF0000}}, // xx001100xxxxxxxx0000000001000110  round.w.s $f0, $f0
{            "sat_s.b WREG , WREG , NUM",{0x0A007078,0xC0FF0700}}, // xx001010xxxxxxxx01110xxx01111000  sat_s.b $w0, $w0, 0
{            "sat_s.d WREG , WREG , NUM",{0x0A000078,0xC0FF3F00}}, // xx001010xxxxxxxx00xxxxxx01111000  sat_s.d $w0, $w0, 0
{            "sat_s.h WREG , WREG , NUM",{0x0A006078,0xC0FF0F00}}, // xx001010xxxxxxxx0110xxxx01111000  sat_s.h $w0, $w0, 0
{            "sat_s.w WREG , WREG , NUM",{0x0A004078,0xC0FF1F00}}, // xx001010xxxxxxxx010xxxxx01111000  sat_s.w $w0, $w0, 0
{            "sat_u.b WREG , WREG , NUM",{0x0A00F078,0xC0FF0700}}, // xx001010xxxxxxxx11110xxx01111000  sat_u.b $w0, $w0, 0
{            "sat_u.d WREG , WREG , NUM",{0x0A008078,0xC0FF3F00}}, // xx001010xxxxxxxx10xxxxxx01111000  sat_u.d $w0, $w0, 0
{            "sat_u.h WREG , WREG , NUM",{0x0A00E078,0xC0FF0F00}}, // xx001010xxxxxxxx1110xxxx01111000  sat_u.h $w0, $w0, 0
{            "sat_u.w WREG , WREG , NUM",{0x0A00C078,0xC0FF1F00}}, // xx001010xxxxxxxx110xxxxx01111000  sat_u.w $w0, $w0, 0
{                 "sb GPREG , ( GPREG )",{0x000000A0,0x0000FF03}}, // 0000000000000000xxxxxxxx101000xx  sb $zero, ($zero)
{             "sb GPREG , NUM ( GPREG )",{0x000100A0,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx101000xx  sb $zero, 0x100($zero)
{                 "sc GPREG , ( GPREG )",{0x2600007C,0x4000FF03}}, // 0x10011000000000xxxxxxxx011111xx  sc $zero, ($zero)
{             "sc GPREG , NUM ( GPREG )",{0x2601007C,0xC0FFFF03}}, // xx100110xxxxxxxxxxxxxxxx011111xx  sc $zero, 2($zero)
{                "scd GPREG , ( GPREG )",{0x2700007C,0x4000FF03}}, // 0x10011100000000xxxxxxxx011111xx  scd $zero, ($zero)
{            "scd GPREG , NUM ( GPREG )",{0x2701007C,0xC0FFFF03}}, // xx100111xxxxxxxxxxxxxxxx011111xx  scd $zero, 2($zero)
{                                "sdbbp",{0x0E000000,0x00000000}}, // 00001110000000000000000000000000  sdbbp
{                            "sdbbp NUM",{0x0E000002,0xC0FFFF03}}, // xx001110xxxxxxxxxxxxxxxx000000xx  sdbbp 0x80000
{                "sdc1 FREG , ( GPREG )",{0x000000F4,0x0000FF03}}, // 0000000000000000xxxxxxxx111101xx  sdc1 $f0, ($zero)
{            "sdc1 FREG , NUM ( GPREG )",{0x000100F4,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx111101xx  sdc1 $f0, 0x100($zero)
{                      "sdc2 CASH , ( )",{0x0000E049,0x00001F00}}, // 0000000000000000111xxxxx01001001  sdc2 $0, ()
{                  "sdc2 CASH , ( NUM )",{0x0001E049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx111xxxxx01001001  sdc2 $0, (0x100)
{                    "seb GPREG , GPREG",{0x2004007C,0x00F81F00}}, // 00100000xxxxx100000xxxxx01111100  seb $zero, $zero
{                    "seh GPREG , GPREG",{0x2006007C,0x00F81F00}}, // 00100000xxxxx110000xxxxx01111100  seh $zero, $zero
{             "sel.d FREG , FREG , FREG",{0x10002046,0xC0FF1F00}}, // xx010000xxxxxxxx001xxxxx01000110  sel.d $f0, $f0, $f0
{             "sel.s FREG , FREG , FREG",{0x10000046,0xC0FF1F00}}, // xx010000xxxxxxxx000xxxxx01000110  sel.s $f0, $f0, $f0
{          "seleqz.d FREG , FREG , FREG",{0x14002046,0xC0FF1F00}}, // xx010100xxxxxxxx001xxxxx01000110  seleqz.d $f0, $f0, $f0
{          "seleqz.s FREG , FREG , FREG",{0x14000046,0xC0FF1F00}}, // xx010100xxxxxxxx000xxxxx01000110  seleqz.s $f0, $f0, $f0
{          "selnez.d FREG , FREG , FREG",{0x17002046,0xC0FF1F00}}, // xx010111xxxxxxxx001xxxxx01000110  selnez.d $f0, $f0, $f0
{          "selnez.s FREG , FREG , FREG",{0x17000046,0xC0FF1F00}}, // xx010111xxxxxxxx000xxxxx01000110  selnez.s $f0, $f0, $f0
{                 "sh GPREG , ( GPREG )",{0x000000A4,0x0000FF03}}, // 0000000000000000xxxxxxxx101001xx  sh $zero, ($zero)
{             "sh GPREG , NUM ( GPREG )",{0x000100A4,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx101001xx  sh $zero, 0x100($zero)
{              "shf.b WREG , WREG , NUM",{0x02000078,0xC0FFFF00}}, // xx000010xxxxxxxxxxxxxxxx01111000  shf.b $w0, $w0, 0
{              "shf.h WREG , WREG , NUM",{0x02000079,0xC0FFFF00}}, // xx000010xxxxxxxxxxxxxxxx01111001  shf.h $w0, $w0, 0
{              "shf.w WREG , WREG , NUM",{0x0200007A,0xC0FFFF00}}, // xx000010xxxxxxxxxxxxxxxx01111010  shf.w $w0, $w0, 0
{                    "shilo ACREG , NUM",{0xB806007C,0x0018F003}}, // 10111000000xx110xxxx0000011111xx  shilo $ac0, 0
{                 "shilov ACREG , GPREG",{0xF806007C,0x0018E003}}, // 11111000000xx110xxx00000011111xx  shilov $ac0, $zero
{          "shll.ph GPREG , GPREG , NUM",{0x1302007C,0x00F8FF03}}, // 00010011xxxxx010xxxxxxxx011111xx  shll.ph $zero, $zero, 0
{          "shll.qb GPREG , GPREG , NUM",{0x1300007C,0x00F8FF03}}, // 00010011xxxxx000xxxxxxxx011111xx  shll.qb $zero, $zero, 0
{        "shll_s.ph GPREG , GPREG , NUM",{0x1303007C,0x00F8FF03}}, // 00010011xxxxx011xxxxxxxx011111xx  shll_s.ph $zero, $zero, 0
{         "shll_s.w GPREG , GPREG , NUM",{0x1305007C,0x00F8FF03}}, // 00010011xxxxx101xxxxxxxx011111xx  shll_s.w $zero, $zero, 0
{       "shllv.ph GPREG , GPREG , GPREG",{0x9302007C,0x00F8FF03}}, // 10010011xxxxx010xxxxxxxx011111xx  shllv.ph $zero, $zero, $zero
{       "shllv.qb GPREG , GPREG , GPREG",{0x9300007C,0x00F8FF03}}, // 10010011xxxxx000xxxxxxxx011111xx  shllv.qb $zero, $zero, $zero
{     "shllv_s.ph GPREG , GPREG , GPREG",{0x9303007C,0x00F8FF03}}, // 10010011xxxxx011xxxxxxxx011111xx  shllv_s.ph $zero, $zero, $zero
{      "shllv_s.w GPREG , GPREG , GPREG",{0x9305007C,0x00F8FF03}}, // 10010011xxxxx101xxxxxxxx011111xx  shllv_s.w $zero, $zero, $zero
{          "shra.ph GPREG , GPREG , NUM",{0x5302007C,0x00F8FF03}}, // 01010011xxxxx010xxxxxxxx011111xx  shra.ph $zero, $zero, 0
{          "shra.qb GPREG , GPREG , NUM",{0x1301007C,0x00F8FF03}}, // 00010011xxxxx001xxxxxxxx011111xx  shra.qb $zero, $zero, 0
{        "shra_r.ph GPREG , GPREG , NUM",{0x5303007C,0x00F8FF03}}, // 01010011xxxxx011xxxxxxxx011111xx  shra_r.ph $zero, $zero, 0
{        "shra_r.qb GPREG , GPREG , NUM",{0x5301007C,0x00F8FF03}}, // 01010011xxxxx001xxxxxxxx011111xx  shra_r.qb $zero, $zero, 0
{         "shra_r.w GPREG , GPREG , NUM",{0x5305007C,0x00F8FF03}}, // 01010011xxxxx101xxxxxxxx011111xx  shra_r.w $zero, $zero, 0
{       "shrav.ph GPREG , GPREG , GPREG",{0xD302007C,0x00F8FF03}}, // 11010011xxxxx010xxxxxxxx011111xx  shrav.ph $zero, $zero, $zero
{       "shrav.qb GPREG , GPREG , GPREG",{0x9301007C,0x00F8FF03}}, // 10010011xxxxx001xxxxxxxx011111xx  shrav.qb $zero, $zero, $zero
{     "shrav_r.ph GPREG , GPREG , GPREG",{0xD303007C,0x00F8FF03}}, // 11010011xxxxx011xxxxxxxx011111xx  shrav_r.ph $zero, $zero, $zero
{     "shrav_r.qb GPREG , GPREG , GPREG",{0xD301007C,0x00F8FF03}}, // 11010011xxxxx001xxxxxxxx011111xx  shrav_r.qb $zero, $zero, $zero
{      "shrav_r.w GPREG , GPREG , GPREG",{0xD305007C,0x00F8FF03}}, // 11010011xxxxx101xxxxxxxx011111xx  shrav_r.w $zero, $zero, $zero
{          "shrl.ph GPREG , GPREG , NUM",{0x5306007C,0x00F8FF03}}, // 01010011xxxxx110xxxxxxxx011111xx  shrl.ph $zero, $zero, 0
{          "shrl.qb GPREG , GPREG , NUM",{0x5300007C,0x00F8FF03}}, // 01010011xxxxx000xxxxxxxx011111xx  shrl.qb $zero, $zero, 0
{       "shrlv.ph GPREG , GPREG , GPREG",{0xD306007C,0x00F8FF03}}, // 11010011xxxxx110xxxxxxxx011111xx  shrlv.ph $zero, $zero, $zero
{       "shrlv.qb GPREG , GPREG , GPREG",{0xD300007C,0x00F8FF03}}, // 11010011xxxxx000xxxxxxxx011111xx  shrlv.qb $zero, $zero, $zero
{          "sld.b WREG , WREG [ GPREG ]",{0x14000078,0xC0FF1F00}}, // xx010100xxxxxxxx000xxxxx01111000  sld.b $w0, $w0[$zero]
{          "sld.d WREG , WREG [ GPREG ]",{0x14006078,0xC0FF1F00}}, // xx010100xxxxxxxx011xxxxx01111000  sld.d $w0, $w0[$zero]
{          "sld.h WREG , WREG [ GPREG ]",{0x14002078,0xC0FF1F00}}, // xx010100xxxxxxxx001xxxxx01111000  sld.h $w0, $w0[$zero]
{          "sld.w WREG , WREG [ GPREG ]",{0x14004078,0xC0FF1F00}}, // xx010100xxxxxxxx010xxxxx01111000  sld.w $w0, $w0[$zero]
{           "sldi.b WREG , WREG [ NUM ]",{0x19000078,0xC0FF0F00}}, // xx011001xxxxxxxx0000xxxx01111000  sldi.b $w0, $w0[0]
{           "sldi.d WREG , WREG [ NUM ]",{0x19003878,0xC0FF0100}}, // xx011001xxxxxxxx0011100x01111000  sldi.d $w0, $w0[0]
{           "sldi.h WREG , WREG [ NUM ]",{0x19002078,0xC0FF0700}}, // xx011001xxxxxxxx00100xxx01111000  sldi.h $w0, $w0[0]
{           "sldi.w WREG , WREG [ NUM ]",{0x19003078,0xC0FF0300}}, // xx011001xxxxxxxx001100xx01111000  sldi.w $w0, $w0[0]
{              "sll GPREG , GPREG , NUM",{0x00000100,0xC0FF1F00}}, // xx000000xxxxxxxx000xxxxx00000000  sll $zero, $at, 0
{             "sll.b WREG , WREG , WREG",{0x0D000078,0xC0FF1F00}}, // xx001101xxxxxxxx000xxxxx01111000  sll.b $w0, $w0, $w0
{             "sll.d WREG , WREG , WREG",{0x0D006078,0xC0FF1F00}}, // xx001101xxxxxxxx011xxxxx01111000  sll.d $w0, $w0, $w0
{             "sll.h WREG , WREG , WREG",{0x0D002078,0xC0FF1F00}}, // xx001101xxxxxxxx001xxxxx01111000  sll.h $w0, $w0, $w0
{             "sll.w WREG , WREG , WREG",{0x0D004078,0xC0FF1F00}}, // xx001101xxxxxxxx010xxxxx01111000  sll.w $w0, $w0, $w0
{             "slli.b WREG , WREG , NUM",{0x09007078,0xC0FF0700}}, // xx001001xxxxxxxx01110xxx01111000  slli.b $w0, $w0, 0
{             "slli.d WREG , WREG , NUM",{0x09000078,0xC0FF3F00}}, // xx001001xxxxxxxx00xxxxxx01111000  slli.d $w0, $w0, 0
{             "slli.h WREG , WREG , NUM",{0x09006078,0xC0FF0F00}}, // xx001001xxxxxxxx0110xxxx01111000  slli.h $w0, $w0, 0
{             "slli.w WREG , WREG , NUM",{0x09004078,0xC0FF1F00}}, // xx001001xxxxxxxx010xxxxx01111000  slli.w $w0, $w0, 0
{           "sllv GPREG , GPREG , GPREG",{0x04000000,0x00F8FF03}}, // 00000100xxxxx000xxxxxxxx000000xx  sllv $zero, $zero, $zero
{            "slt GPREG , GPREG , GPREG",{0x2A000000,0x00F8FF03}}, // 00101010xxxxx000xxxxxxxx000000xx  slt $zero, $zero, $zero
{             "slti GPREG , GPREG , NUM",{0x00000028,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001010xx  slti $zero, $zero, 0
{            "sltiu GPREG , GPREG , NUM",{0x0000002C,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001011xx  sltiu $zero, $zero, 0
{           "sltu GPREG , GPREG , GPREG",{0x2B000000,0x00F8FF03}}, // 00101011xxxxx000xxxxxxxx000000xx  sltu $zero, $zero, $zero
{        "splat.b WREG , WREG [ GPREG ]",{0x14008078,0xC0FF1F00}}, // xx010100xxxxxxxx100xxxxx01111000  splat.b $w0, $w0[$zero]
{        "splat.d WREG , WREG [ GPREG ]",{0x1400E078,0xC0FF1F00}}, // xx010100xxxxxxxx111xxxxx01111000  splat.d $w0, $w0[$zero]
{        "splat.h WREG , WREG [ GPREG ]",{0x1400A078,0xC0FF1F00}}, // xx010100xxxxxxxx101xxxxx01111000  splat.h $w0, $w0[$zero]
{        "splat.w WREG , WREG [ GPREG ]",{0x1400C078,0xC0FF1F00}}, // xx010100xxxxxxxx110xxxxx01111000  splat.w $w0, $w0[$zero]
{         "splati.b WREG , WREG [ NUM ]",{0x19004078,0xC0FF0F00}}, // xx011001xxxxxxxx0100xxxx01111000  splati.b $w0, $w0[0]
{         "splati.d WREG , WREG [ NUM ]",{0x19007878,0xC0FF0100}}, // xx011001xxxxxxxx0111100x01111000  splati.d $w0, $w0[0]
{         "splati.h WREG , WREG [ NUM ]",{0x19006078,0xC0FF0700}}, // xx011001xxxxxxxx01100xxx01111000  splati.h $w0, $w0[0]
{         "splati.w WREG , WREG [ NUM ]",{0x19007078,0xC0FF0300}}, // xx011001xxxxxxxx011100xx01111000  splati.w $w0, $w0[0]
{                   "sqrt.d FREG , FREG",{0x04002046,0x80F70000}}, // x0000100xxxx0xxx0010000001000110  sqrt.d $f0, $f0
{                   "sqrt.s FREG , FREG",{0x04000046,0xC0FF0000}}, // xx000100xxxxxxxx0000000001000110  sqrt.s $f0, $f0
{              "sra GPREG , GPREG , NUM",{0x03000000,0xC0FF1F00}}, // xx000011xxxxxxxx000xxxxx00000000  sra $zero, $zero, 0
{             "sra.b WREG , WREG , WREG",{0x0D008078,0xC0FF1F00}}, // xx001101xxxxxxxx100xxxxx01111000  sra.b $w0, $w0, $w0
{             "sra.d WREG , WREG , WREG",{0x0D00E078,0xC0FF1F00}}, // xx001101xxxxxxxx111xxxxx01111000  sra.d $w0, $w0, $w0
{             "sra.h WREG , WREG , WREG",{0x0D00A078,0xC0FF1F00}}, // xx001101xxxxxxxx101xxxxx01111000  sra.h $w0, $w0, $w0
{             "sra.w WREG , WREG , WREG",{0x0D00C078,0xC0FF1F00}}, // xx001101xxxxxxxx110xxxxx01111000  sra.w $w0, $w0, $w0
{             "srai.b WREG , WREG , NUM",{0x0900F078,0xC0FF0700}}, // xx001001xxxxxxxx11110xxx01111000  srai.b $w0, $w0, 0
{             "srai.d WREG , WREG , NUM",{0x09008078,0xC0FF3F00}}, // xx001001xxxxxxxx10xxxxxx01111000  srai.d $w0, $w0, 0
{             "srai.h WREG , WREG , NUM",{0x0900E078,0xC0FF0F00}}, // xx001001xxxxxxxx1110xxxx01111000  srai.h $w0, $w0, 0
{             "srai.w WREG , WREG , NUM",{0x0900C078,0xC0FF1F00}}, // xx001001xxxxxxxx110xxxxx01111000  srai.w $w0, $w0, 0
{            "srar.b WREG , WREG , WREG",{0x15008078,0xC0FF1F00}}, // xx010101xxxxxxxx100xxxxx01111000  srar.b $w0, $w0, $w0
{            "srar.d WREG , WREG , WREG",{0x1500E078,0xC0FF1F00}}, // xx010101xxxxxxxx111xxxxx01111000  srar.d $w0, $w0, $w0
{            "srar.h WREG , WREG , WREG",{0x1500A078,0xC0FF1F00}}, // xx010101xxxxxxxx101xxxxx01111000  srar.h $w0, $w0, $w0
{            "srar.w WREG , WREG , WREG",{0x1500C078,0xC0FF1F00}}, // xx010101xxxxxxxx110xxxxx01111000  srar.w $w0, $w0, $w0
{            "srari.b WREG , WREG , NUM",{0x0A007079,0xC0FF0700}}, // xx001010xxxxxxxx01110xxx01111001  srari.b $w0, $w0, 0
{            "srari.d WREG , WREG , NUM",{0x0A000079,0xC0FF3F00}}, // xx001010xxxxxxxx00xxxxxx01111001  srari.d $w0, $w0, 0
{            "srari.h WREG , WREG , NUM",{0x0A006079,0xC0FF0F00}}, // xx001010xxxxxxxx0110xxxx01111001  srari.h $w0, $w0, 0
{            "srari.w WREG , WREG , NUM",{0x0A004079,0xC0FF1F00}}, // xx001010xxxxxxxx010xxxxx01111001  srari.w $w0, $w0, 0
{           "srav GPREG , GPREG , GPREG",{0x07000000,0x00F8FF03}}, // 00000111xxxxx000xxxxxxxx000000xx  srav $zero, $zero, $zero
{              "srl GPREG , GPREG , NUM",{0x02000000,0xC0FF1F00}}, // xx000010xxxxxxxx000xxxxx00000000  srl $zero, $zero, 0
{             "srl.b WREG , WREG , WREG",{0x0D000079,0xC0FF1F00}}, // xx001101xxxxxxxx000xxxxx01111001  srl.b $w0, $w0, $w0
{             "srl.d WREG , WREG , WREG",{0x0D006079,0xC0FF1F00}}, // xx001101xxxxxxxx011xxxxx01111001  srl.d $w0, $w0, $w0
{             "srl.h WREG , WREG , WREG",{0x0D002079,0xC0FF1F00}}, // xx001101xxxxxxxx001xxxxx01111001  srl.h $w0, $w0, $w0
{             "srl.w WREG , WREG , WREG",{0x0D004079,0xC0FF1F00}}, // xx001101xxxxxxxx010xxxxx01111001  srl.w $w0, $w0, $w0
{             "srli.b WREG , WREG , NUM",{0x09007079,0xC0FF0700}}, // xx001001xxxxxxxx01110xxx01111001  srli.b $w0, $w0, 0
{             "srli.d WREG , WREG , NUM",{0x09000079,0xC0FF3F00}}, // xx001001xxxxxxxx00xxxxxx01111001  srli.d $w0, $w0, 0
{             "srli.h WREG , WREG , NUM",{0x09006079,0xC0FF0F00}}, // xx001001xxxxxxxx0110xxxx01111001  srli.h $w0, $w0, 0
{             "srli.w WREG , WREG , NUM",{0x09004079,0xC0FF1F00}}, // xx001001xxxxxxxx010xxxxx01111001  srli.w $w0, $w0, 0
{            "srlr.b WREG , WREG , WREG",{0x15000079,0xC0FF1F00}}, // xx010101xxxxxxxx000xxxxx01111001  srlr.b $w0, $w0, $w0
{            "srlr.d WREG , WREG , WREG",{0x15006079,0xC0FF1F00}}, // xx010101xxxxxxxx011xxxxx01111001  srlr.d $w0, $w0, $w0
{            "srlr.h WREG , WREG , WREG",{0x15002079,0xC0FF1F00}}, // xx010101xxxxxxxx001xxxxx01111001  srlr.h $w0, $w0, $w0
{            "srlr.w WREG , WREG , WREG",{0x15004079,0xC0FF1F00}}, // xx010101xxxxxxxx010xxxxx01111001  srlr.w $w0, $w0, $w0
{            "srlri.b WREG , WREG , NUM",{0x0A00F079,0xC0FF0700}}, // xx001010xxxxxxxx11110xxx01111001  srlri.b $w0, $w0, 0
{            "srlri.d WREG , WREG , NUM",{0x0A008079,0xC0FF3F00}}, // xx001010xxxxxxxx10xxxxxx01111001  srlri.d $w0, $w0, 0
{            "srlri.h WREG , WREG , NUM",{0x0A00E079,0xC0FF0F00}}, // xx001010xxxxxxxx1110xxxx01111001  srlri.h $w0, $w0, 0
{            "srlri.w WREG , WREG , NUM",{0x0A00C079,0xC0FF1F00}}, // xx001010xxxxxxxx110xxxxx01111001  srlri.w $w0, $w0, 0
{           "srlv GPREG , GPREG , GPREG",{0x06000000,0x00F8FF03}}, // 00000110xxxxx000xxxxxxxx000000xx  srlv $zero, $zero, $zero
{                                "ssnop",{0x40000000,0x00000000}}, // 01000000000000000000000000000000  ssnop
{                "st.b WREG , ( GPREG )",{0x24000078,0xC0FF0000}}, // xx100100xxxxxxxx0000000001111000  st.b $w0, ($zero)
{            "st.b WREG , NUM ( GPREG )",{0x2400007A,0xC0FFFF03}}, // xx100100xxxxxxxxxxxxxxxx011110xx  st.b $w0, -0x200($zero)
{                "st.d WREG , ( GPREG )",{0x27000078,0xC0FF0000}}, // xx100111xxxxxxxx0000000001111000  st.d $w0, ($zero)
{            "st.d WREG , NUM ( GPREG )",{0x2700007A,0xC0FFFF03}}, // xx100111xxxxxxxxxxxxxxxx011110xx  st.d $w0, -0x1000($zero)
{                "st.h WREG , ( GPREG )",{0x25000078,0xC0FF0000}}, // xx100101xxxxxxxx0000000001111000  st.h $w0, ($zero)
{            "st.h WREG , NUM ( GPREG )",{0x2500007A,0xC0FFFF03}}, // xx100101xxxxxxxxxxxxxxxx011110xx  st.h $w0, -0x400($zero)
{                "st.w WREG , ( GPREG )",{0x26000078,0xC0FF0000}}, // xx100110xxxxxxxx0000000001111000  st.w $w0, ($zero)
{            "st.w WREG , NUM ( GPREG )",{0x2600007A,0xC0FFFF03}}, // xx100110xxxxxxxxxxxxxxxx011110xx  st.w $w0, -0x800($zero)
{            "sub GPREG , GPREG , GPREG",{0x22000001,0x00F8FF03}}, // 00100010xxxxx000xxxxxxxx000000xx  sub $zero, $t0, $zero
{             "sub.d FREG , FREG , FREG",{0x01002046,0x80F71E00}}, // x0000001xxxx0xxx001xxxx001000110  sub.d $f0, $f0, $f0
{             "sub.s FREG , FREG , FREG",{0x01000046,0xC0FF1F00}}, // xx000001xxxxxxxx000xxxxx01000110  sub.s $f0, $f0, $f0
{        "subq.ph GPREG , GPREG , GPREG",{0xD002007C,0x00F8FF03}}, // 11010000xxxxx010xxxxxxxx011111xx  subq.ph $zero, $zero, $zero
{      "subq_s.ph GPREG , GPREG , GPREG",{0xD003007C,0x00F8FF03}}, // 11010000xxxxx011xxxxxxxx011111xx  subq_s.ph $zero, $zero, $zero
{       "subq_s.w GPREG , GPREG , GPREG",{0xD005007C,0x00F8FF03}}, // 11010000xxxxx101xxxxxxxx011111xx  subq_s.w $zero, $zero, $zero
{       "subqh.ph GPREG , GPREG , GPREG",{0x5802007C,0x00F8FF03}}, // 01011000xxxxx010xxxxxxxx011111xx  subqh.ph $zero, $zero, $zero
{        "subqh.w GPREG , GPREG , GPREG",{0x5804007C,0x00F8FF03}}, // 01011000xxxxx100xxxxxxxx011111xx  subqh.w $zero, $zero, $zero
{     "subqh_r.ph GPREG , GPREG , GPREG",{0xD802007C,0x00F8FF03}}, // 11011000xxxxx010xxxxxxxx011111xx  subqh_r.ph $zero, $zero, $zero
{      "subqh_r.w GPREG , GPREG , GPREG",{0xD804007C,0x00F8FF03}}, // 11011000xxxxx100xxxxxxxx011111xx  subqh_r.w $zero, $zero, $zero
{          "subs_s.b WREG , WREG , WREG",{0x11000078,0xC0FF1F00}}, // xx010001xxxxxxxx000xxxxx01111000  subs_s.b $w0, $w0, $w0
{          "subs_s.d WREG , WREG , WREG",{0x11006078,0xC0FF1F00}}, // xx010001xxxxxxxx011xxxxx01111000  subs_s.d $w0, $w0, $w0
{          "subs_s.h WREG , WREG , WREG",{0x11002078,0xC0FF1F00}}, // xx010001xxxxxxxx001xxxxx01111000  subs_s.h $w0, $w0, $w0
{          "subs_s.w WREG , WREG , WREG",{0x11004078,0xC0FF1F00}}, // xx010001xxxxxxxx010xxxxx01111000  subs_s.w $w0, $w0, $w0
{          "subs_u.b WREG , WREG , WREG",{0x11008078,0xC0FF1F00}}, // xx010001xxxxxxxx100xxxxx01111000  subs_u.b $w0, $w0, $w0
{          "subs_u.d WREG , WREG , WREG",{0x1100E078,0xC0FF1F00}}, // xx010001xxxxxxxx111xxxxx01111000  subs_u.d $w0, $w0, $w0
{          "subs_u.h WREG , WREG , WREG",{0x1100A078,0xC0FF1F00}}, // xx010001xxxxxxxx101xxxxx01111000  subs_u.h $w0, $w0, $w0
{          "subs_u.w WREG , WREG , WREG",{0x1100C078,0xC0FF1F00}}, // xx010001xxxxxxxx110xxxxx01111000  subs_u.w $w0, $w0, $w0
{        "subsus_u.b WREG , WREG , WREG",{0x11000079,0xC0FF1F00}}, // xx010001xxxxxxxx000xxxxx01111001  subsus_u.b $w0, $w0, $w0
{        "subsus_u.d WREG , WREG , WREG",{0x11006079,0xC0FF1F00}}, // xx010001xxxxxxxx011xxxxx01111001  subsus_u.d $w0, $w0, $w0
{        "subsus_u.h WREG , WREG , WREG",{0x11002079,0xC0FF1F00}}, // xx010001xxxxxxxx001xxxxx01111001  subsus_u.h $w0, $w0, $w0
{        "subsus_u.w WREG , WREG , WREG",{0x11004079,0xC0FF1F00}}, // xx010001xxxxxxxx010xxxxx01111001  subsus_u.w $w0, $w0, $w0
{        "subsuu_s.b WREG , WREG , WREG",{0x11008079,0xC0FF1F00}}, // xx010001xxxxxxxx100xxxxx01111001  subsuu_s.b $w0, $w0, $w0
{        "subsuu_s.d WREG , WREG , WREG",{0x1100E079,0xC0FF1F00}}, // xx010001xxxxxxxx111xxxxx01111001  subsuu_s.d $w0, $w0, $w0
{        "subsuu_s.h WREG , WREG , WREG",{0x1100A079,0xC0FF1F00}}, // xx010001xxxxxxxx101xxxxx01111001  subsuu_s.h $w0, $w0, $w0
{        "subsuu_s.w WREG , WREG , WREG",{0x1100C079,0xC0FF1F00}}, // xx010001xxxxxxxx110xxxxx01111001  subsuu_s.w $w0, $w0, $w0
{           "subu GPREG , GPREG , GPREG",{0x23000001,0x00F8FF03}}, // 00100011xxxxx000xxxxxxxx000000xx  subu $zero, $t0, $zero
{        "subu.ph GPREG , GPREG , GPREG",{0x5002007C,0x00F8FF03}}, // 01010000xxxxx010xxxxxxxx011111xx  subu.ph $zero, $zero, $zero
{        "subu.qb GPREG , GPREG , GPREG",{0x5000007C,0x00F8FF03}}, // 01010000xxxxx000xxxxxxxx011111xx  subu.qb $zero, $zero, $zero
{      "subu_s.ph GPREG , GPREG , GPREG",{0x5003007C,0x00F8FF03}}, // 01010000xxxxx011xxxxxxxx011111xx  subu_s.ph $zero, $zero, $zero
{      "subu_s.qb GPREG , GPREG , GPREG",{0x5001007C,0x00F8FF03}}, // 01010000xxxxx001xxxxxxxx011111xx  subu_s.qb $zero, $zero, $zero
{       "subuh.qb GPREG , GPREG , GPREG",{0x5800007C,0x00F8FF03}}, // 01011000xxxxx000xxxxxxxx011111xx  subuh.qb $zero, $zero, $zero
{     "subuh_r.qb GPREG , GPREG , GPREG",{0xD800007C,0x00F8FF03}}, // 11011000xxxxx000xxxxxxxx011111xx  subuh_r.qb $zero, $zero, $zero
{            "subv.b WREG , WREG , WREG",{0x0E008078,0xC0FF1F00}}, // xx001110xxxxxxxx100xxxxx01111000  subv.b $w0, $w0, $w0
{            "subv.d WREG , WREG , WREG",{0x0E00E078,0xC0FF1F00}}, // xx001110xxxxxxxx111xxxxx01111000  subv.d $w0, $w0, $w0
{            "subv.h WREG , WREG , WREG",{0x0E00A078,0xC0FF1F00}}, // xx001110xxxxxxxx101xxxxx01111000  subv.h $w0, $w0, $w0
{            "subv.w WREG , WREG , WREG",{0x0E00C078,0xC0FF1F00}}, // xx001110xxxxxxxx110xxxxx01111000  subv.w $w0, $w0, $w0
{            "subvi.b WREG , WREG , NUM",{0x06008078,0xC0FF1F00}}, // xx000110xxxxxxxx100xxxxx01111000  subvi.b $w0, $w0, 0
{            "subvi.d WREG , WREG , NUM",{0x0600E078,0xC0FF1F00}}, // xx000110xxxxxxxx111xxxxx01111000  subvi.d $w0, $w0, 0
{            "subvi.h WREG , WREG , NUM",{0x0600A078,0xC0FF1F00}}, // xx000110xxxxxxxx101xxxxx01111000  subvi.h $w0, $w0, 0
{            "subvi.w WREG , WREG , NUM",{0x0600C078,0xC0FF1F00}}, // xx000110xxxxxxxx110xxxxx01111000  subvi.w $w0, $w0, 0
{                 "sw GPREG , ( GPREG )",{0x000000AC,0x0000FF03}}, // 0000000000000000xxxxxxxx101011xx  sw $zero, ($zero)
{             "sw GPREG , NUM ( GPREG )",{0x000100AC,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx101011xx  sw $zero, 0x100($zero)
{                "swc1 FREG , ( GPREG )",{0x000000E4,0x0000FF03}}, // 0000000000000000xxxxxxxx111001xx  swc1 $f0, ($zero)
{            "swc1 FREG , NUM ( GPREG )",{0x000100E4,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx111001xx  swc1 $f0, 0x100($zero)
{                      "swc2 CASH , ( )",{0x00006049,0x00001F00}}, // 0000000000000000011xxxxx01001001  swc2 $0, ()
{                  "swc2 CASH , ( NUM )",{0x00016049,0xFFFF1F00}}, // xxxxxxxxxxxxxxxx011xxxxx01001001  swc2 $0, (0x100)
{                                 "sync",{0x0F000000,0x00F8FF03}}, // 00001111xxxxx000xxxxxxxx000000xx  sync
{                             "sync NUM",{0x0F010000,0xC0FFFF03}}, // xx001111xxxxxxxxxxxxxxxx000000xx  sync 4
{                              "syscall",{0x0C000000,0x00000000}}, // 00001100000000000000000000000000  syscall
{                          "syscall NUM",{0x0C000002,0xC0FFFF03}}, // xx001100xxxxxxxxxxxxxxxx000000xx  syscall 0x80000
{                    "teq GPREG , GPREG",{0x34000000,0x0000FF03}}, // 0011010000000000xxxxxxxx000000xx  teq $zero, $zero
{              "teq GPREG , GPREG , NUM",{0x34010000,0xC0FFFF03}}, // xx110100xxxxxxxxxxxxxxxx000000xx  teq $zero, $zero, 4
{                    "tge GPREG , GPREG",{0x30000000,0x0000FF03}}, // 0011000000000000xxxxxxxx000000xx  tge $zero, $zero
{              "tge GPREG , GPREG , NUM",{0x30010000,0xC0FFFF03}}, // xx110000xxxxxxxxxxxxxxxx000000xx  tge $zero, $zero, 4
{                   "tgeu GPREG , GPREG",{0x31000000,0x0000FF03}}, // 0011000100000000xxxxxxxx000000xx  tgeu $zero, $zero
{             "tgeu GPREG , GPREG , NUM",{0x31010000,0xC0FFFF03}}, // xx110001xxxxxxxxxxxxxxxx000000xx  tgeu $zero, $zero, 4
{                                 "tlbp",{0x08000042,0x00000000}}, // 00001000000000000000000001000010  tlbp
{                                 "tlbr",{0x01000042,0x00000000}}, // 00000001000000000000000001000010  tlbr
{                                "tlbwi",{0x02000042,0x00000000}}, // 00000010000000000000000001000010  tlbwi
{                                "tlbwr",{0x06000042,0x00000000}}, // 00000110000000000000000001000010  tlbwr
{                    "tlt GPREG , GPREG",{0x32000000,0x0000FF03}}, // 0011001000000000xxxxxxxx000000xx  tlt $zero, $zero
{              "tlt GPREG , GPREG , NUM",{0x32010000,0xC0FFFF03}}, // xx110010xxxxxxxxxxxxxxxx000000xx  tlt $zero, $zero, 4
{                   "tltu GPREG , GPREG",{0x33000000,0x0000FF03}}, // 0011001100000000xxxxxxxx000000xx  tltu $zero, $zero
{             "tltu GPREG , GPREG , NUM",{0x33010000,0xC0FFFF03}}, // xx110011xxxxxxxxxxxxxxxx000000xx  tltu $zero, $zero, 4
{                    "tne GPREG , GPREG",{0x36000000,0x0000FF03}}, // 0011011000000000xxxxxxxx000000xx  tne $zero, $zero
{              "tne GPREG , GPREG , NUM",{0x36010000,0xC0FFFF03}}, // xx110110xxxxxxxxxxxxxxxx000000xx  tne $zero, $zero, 4
{                "trunc.w.d FREG , FREG",{0x0D002046,0xC0F70000}}, // xx001101xxxx0xxx0010000001000110  trunc.w.d $f0, $f0
{                "trunc.w.s FREG , FREG",{0x0D000046,0xC0FF0000}}, // xx001101xxxxxxxx0000000001000110  trunc.w.s $f0, $f0
{                                "undef",{0x00000001,0xFFFFFFFF}}, // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  undef
{            "vshf.b WREG , WREG , WREG",{0x15000078,0xC0FF1F00}}, // xx010101xxxxxxxx000xxxxx01111000  vshf.b $w0, $w0, $w0
{            "vshf.d WREG , WREG , WREG",{0x15006078,0xC0FF1F00}}, // xx010101xxxxxxxx011xxxxx01111000  vshf.d $w0, $w0, $w0
{            "vshf.h WREG , WREG , WREG",{0x15002078,0xC0FF1F00}}, // xx010101xxxxxxxx001xxxxx01111000  vshf.h $w0, $w0, $w0
{            "vshf.w WREG , WREG , WREG",{0x15004078,0xC0FF1F00}}, // xx010101xxxxxxxx010xxxxx01111000  vshf.w $w0, $w0, $w0
{                                 "wait",{0x20000042,0x00000000}}, // 00100000000000000000000001000010  wait
{                    "wrdsp GPREG , NUM",{0xF804007C,0x00F8FF03}}, // 11111000xxxxx100xxxxxxxx011111xx  wrdsp $zero, 0
{                   "wsbh GPREG , GPREG",{0xA000007C,0x00F81F00}}, // 10100000xxxxx000000xxxxx01111100  wsbh $zero, $zero
{            "xor GPREG , GPREG , GPREG",{0x26000000,0x00F8FF03}}, // 00100110xxxxx000xxxxxxxx000000xx  xor $zero, $zero, $zero
{             "xor.v WREG , WREG , WREG",{0x1E006078,0xC0FF1F00}}, // xx011110xxxxxxxx011xxxxx01111000  xor.v $w0, $w0, $w0
{             "xori GPREG , GPREG , NUM",{0x00000038,0xFFFFFF03}}, // xxxxxxxxxxxxxxxxxxxxxxxx001110xx  xori $zero, $zero, 0
{             "xori.b WREG , WREG , NUM",{0x0000007B,0xC0FFFF00}}, // xx000000xxxxxxxxxxxxxxxx01111011  xori.b $w0, $w0, 0
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
		cs_mode mode = (cs_mode)(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32R6);

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
	string sig_src = tokens_to_syntax(toks_src);
	
	//printf("src:%s has syntax:%s\n", src.c_str(), sig_src.c_str());

	if(lookup.find(sig_src) == lookup.end()) {
		err = "invalid syntax (tried to look up: \"" + sig_src + "\")";
		return -1;
	}

	auto info = lookup[sig_src];
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
