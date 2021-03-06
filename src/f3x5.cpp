/* f3x5.cpp

RAWRR!

*/

/*
	MACROS
*/
#define HEX__(n)		(0x##n##U)
#define B4__(x)			(((x&0x000FU)?1:0) | ((x&0x00F0U)?2:0) \
						|((x&0x0F00U)?4:0) | ((x&0xF000U)?8:0))
#define B4(b)			static_cast<unsigned short>(B4__(HEX__(b)))
#define L1(b)			(B4(b) << 0x0)
#define L2(b)			(B4(b) << 0x3)
#define L3(b)			(B4(b) << 0x6)
#define L4(b)			(B4(b) << 0x9)
#define L5(b)			(B4(b) << 0xC)

/*
	f3x5[]

	each symbol is 3x5 => 15 bits
	
	a-z		offset 0	length 26
	0-9		offset 26	length 10
	#		offset 36	length 1
*/
unsigned short f3x5[] = {
	// A
	L1(011) |
	L2(101) |
	L3(101) |
	L4(111) |
	L5(101)
	,
	// B
	L1(110) |
	L2(101) |
	L3(110) |
	L4(101) |
	L5(110)
	,
	// C
	L1(011) |
	L2(101) |
	L3(100) |
	L4(100) |
	L5(011)
	,
	// D
	L1(110) |
	L2(101) |
	L3(101) |
	L4(101) |
	L5(110)
	,
	// E
	L1(011) |
	L2(100) |
	L3(110) |
	L4(100) |
	L5(110)
	,
	// F
	L1(011) |
	L2(100) |
	L3(110) |
	L4(100) |
	L5(100)
	,
	// G
	L1(011) |
	L2(100) |
	L3(101) |
	L4(101) |
	L5(110)
	,
	// H
	L1(101) |
	L2(101) |
	L3(101) |
	L4(111) |
	L5(101)
	,
	// I
	L1(111) |
	L2(010) |
	L3(010) |
	L4(010) |
	L5(111)
	,
	// J
	L1(111) |
	L2(001) |
	L3(001) |
	L4(001) |
	L5(110)
	,
	// K
	L1(101) |
	L2(101) |
	L3(101) |
	L4(110) |
	L5(101)
	,
	// L
	L1(100) |
	L2(100) |
	L3(100) |
	L4(100) |
	L5(011)
	,
	// M
	L1(101) |
	L2(111) |
	L3(101) |
	L4(101) |
	L5(101)
	,
	// N
	L1(110) |
	L2(101) |
	L3(101) |
	L4(101) |
	L5(101)
	,
	// O
	L1(011) |
	L2(101) |
	L3(101) |
	L4(101) |
	L5(110)
	,
	// P
	L1(110) |
	L2(101) |
	L3(110) |
	L4(100) |
	L5(100)
	,
	// Q
	L1(111) |
	L2(101) |
	L3(111) |
	L4(001) |
	L5(001)
	,
	// R
	L1(110) |
	L2(101) |
	L3(101) |
	L4(110) |
	L5(101)
	,
	// S
	L1(011) |
	L2(100) |
	L3(010) |
	L4(001) |
	L5(110)
	,
	// T
	L1(111) |
	L2(010) |
	L3(010) |
	L4(010) |
	L5(010)
	,
	// U
	L1(101) |
	L2(101) |
	L3(101) |
	L4(101) |
	L5(011)
	,
	// V
	L1(101) |
	L2(101) |
	L3(101) |
	L4(010) |
	L5(010)
	,
	// W
	L1(101) |
	L2(101) |
	L3(101) |
	L4(111) |
	L5(111)
	,
	// X
	L1(101) |
	L2(101) |
	L3(010) |
	L4(101) |
	L5(101)
	,
	// Y
	L1(101) |
	L2(101) |
	L3(011) |
	L4(001) |
	L5(001)
	,
	// Z
	L1(111) |
	L2(001) |
	L3(010) |
	L4(100) |
	L5(111)
	,
	// 0
	L1(011) |
	L2(101) |
	L3(111) |
	L4(101) |
	L5(110)
	,
	// 1
	L1(010) |
	L2(110) |
	L3(010) |
	L4(010) |
	L5(111)
	,
	// 2
	L1(110) |
	L2(101) |
	L3(001) |
	L4(010) |
	L5(111)
	,
	// 3
	L1(110) |
	L2(001) |
	L3(010) |
	L4(001) |
	L5(110)
	,
	// 4
	L1(001) |
	L2(011) |
	L3(101) |
	L4(111) |
	L5(001)
	,
	// 5
	L1(111) |
	L2(100) |
	L3(011) |
	L4(001) |
	L5(110)
	,
	// 6
	L1(011) |
	L2(100) |
	L3(111) |
	L4(101) |
	L5(111)
	,
	// 7
	L1(111) |
	L2(101) |
	L3(001) |
	L4(010) |
	L5(010)
	,
	// 8
	L1(011) |
	L2(101) |
	L3(010) |
	L4(101) |
	L5(110)
	,
	// 9
	L1(010) |
	L2(101) |
	L3(101) |
	L4(011) |
	L5(001)
	,
	// #
	L1(111) |
	L2(111) |
	L3(111) |
	L4(111) |
	L5(111)
};

/*
	f3x5t[]

	this table maps ascii codes to symbol indices
*/
unsigned char f3x5t[] =
{
//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	26,	27,	28,	29,	30,	31,	32,	33,	34,	35,	36,	36,	36,	36, 36, 36,	// 0-9
	36,	0 ,	1 ,	2 ,	3 ,	4 ,	5 ,	6 ,	7 ,	8 ,	9 ,	10,	11,	12, 13, 14,	// A-Z
	15,	16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	36,	36,	36, 36, 36,
	36,	0 ,	1 ,	2 ,	3 ,	4 ,	5 ,	6 ,	7 ,	8 ,	9 ,	10,	11,	12, 13, 14,	// a-z
	15,	16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	36,	36,	36, 36, 36,

	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36,
	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36,	36, 36, 36
};

