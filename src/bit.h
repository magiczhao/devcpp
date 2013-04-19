#ifndef _DEVLIB_BIT_H
#define _DEVLIB_BIT_H
/** bit operations */

/** 
 * count 1's in the number, time is O(n) which n is the result ,
 * this function is faster when n is small
 */
unsigned int bits_set_smallnum(unsigned long number);
/**
 * count 1's in the number, by searching a table, time is O(1)
 * this function is faster when n is large
 */
unsigned int bits_set_largenum(unsigned long number);

/** shortcuts for bits_set */
#ifdef HAVE_LARGE_NUMBER
#define bits_set    bits_set_largenum
#else
#define bits_set    bits_set_smallnum
#endif

/**
 * set the bit'th bit of ptr to 1
 */
#define set_bit(ptr, bit)   do{*(ptr) |= (1 << bit);}while(0)
/**
 * set the bit'th bit of ptr to 0
 */
#define clear_bit(ptr, bit) do{*(ptr) ^= (1 << bit);}while(0)
/** 
 * get the value of the bit's bit
 */
#define is_bit_set(ptr, bit) (*(ptr) & (1 << bit))
#endif
