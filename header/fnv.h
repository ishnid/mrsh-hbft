/* 
 * File:   util.h
 *
 * Created on 5. Juni 2012, 14:38
 */
#ifndef UTIL_H
#define	UTIL_H
#include <stdio.h>
#include <stdint.h>
#include "../header/config.h"



uint64	 		fnv64Bit( unsigned char pBuffer[], int start, int end);
unsigned int	*fnv256Bit(unsigned char pBuffer[], int start, int end);
int 			*mulWithPrime(uint256 hash_val); //),« uint256 result);
void 			mulWithPrime2(uint256 hash_val, uint256 result);
//void 			mulWithPrime3(uint256 hash_val, uint256 tmp_hash);

#endif	/* UTIL_H */

