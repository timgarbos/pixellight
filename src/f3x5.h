/* f3x5.h

RAWRR!

*/

#ifndef __F3X5_H__
#define __F3X5_H__

/*
	f3x5[]

	each symbol is 3x5 => 15 bits
	
	a-z		offset 0	length 26
	0-9		offset 26	length 10
	#		offset 36	length 1
*/
extern unsigned short f3x5[];

/*
	f3x5t[]

	this table maps ascii codes to symbol indices
*/
extern unsigned char f3x5t[];

#endif//__F3X5_H__