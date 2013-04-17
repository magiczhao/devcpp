#ifndef _DEVLIB_BIT_H
#define _DEVLIB_BIT_H
unsigned int bits_set_smallnum(unsigned long number);
unsigned int bits_set_largenum(unsigned long number);

#ifdef HAVE_LARGE_NUMBER
#define bits_set    bits_set_largenum
#else
#define bits_set    bits_set_smallnum
#endif

#endif