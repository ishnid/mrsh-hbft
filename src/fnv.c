#include "../header/fnv.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>





uint64 fnv64Bit( unsigned char pBuffer[], int start, int end)
 {
   uint64 nHashVal    = 0xcbf29ce484222325ULL,
          nMagicPrime = 0x00000100000001b3ULL;

   int i = start;
   while( i <= end ) {
	   nHashVal ^= pBuffer[i++];
	   nHashVal *= nMagicPrime;
   }
   return nHashVal;
 }

/*void fnv256Bit_old(unsigned char pBuffer[], int start, int end, mpz_t nHashVal){
//	mpz_t nHashVal;
	mpz_t nMagicPrime;
	mpz_t bit256;
	mpz_t c; mpz_init2 (c, 8);

	//short value down to 256bit
	mpz_init_set_str(bit256, "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 0);

	//Initial value
	mpz_init_set_str(nHashVal, "0xDD268DBCAAC550362D98C384C4E576CCC8B1536847B6BBB31023B4C8CAEE0535", 0);

	//Magic Prime
	mpz_init_set_str(nMagicPrime, "0x1000000000000000000000000000000000000000163", 0);

	mpz_mul(nHashVal, nHashVal, nMagicPrime);
	mpz_and(nHashVal, nHashVal, bit256);

	int i = start;
	while( i <= end ) {
		mpz_set_ui (c, (int)pBuffer[i++]);
		//nHashVal ^= pBuffer[i++];
		mpz_xor(nHashVal, nHashVal, c);
		//nHashVal *= nMagicPrime;
		mpz_mul(nHashVal, nHashVal, nMagicPrime);
		mpz_and(nHashVal, nHashVal, bit256);
	}

	// free used memory
	mpz_clear(nMagicPrime);
	mpz_clear(bit256);
	//gmp_printf("x: %Zx\n", nHashVal);
	//return nHashVal;

}*/


unsigned int *fnv256Bit(unsigned char pBuffer[], int start, int end){
	unsigned int *hash = (int*)malloc(sizeof(uint256));
	hash[0] = 0xCAEE0535;
	hash[1] = 0x1023B4C8;
	hash[2] = 0x47B6BBB3;
	hash[3] = 0xC8B15368;
	hash[4] = 0xC4E576CC;
	hash[5] = 0x2D98C384;
	hash[6] = 0xAAC55036;
	hash[7] = 0xDD268DBC;

	unsigned int *result = malloc(sizeof(uint256));
	int *swap;

	int i = start;
	while( i <= end ) {
		hash[0] = hash[0] ^ pBuffer[i++];
		mulWithPrime2(hash, result);
		swap = hash;
		hash = result;
		result = swap;
	}

	free(result);
	return hash;
}




//mul with magic prime: 0x1000000000000000000000000000000000000000163 = 2^168 + 2^8 + 0x63
//0x63 = 0110 0011
void mulWithPrime2(uint256 hash_val, uint256 result){
	uint64 tmp = 0;

	/* the 256 bit hash is stored as 32-bit integers, to handle the overflow we store them in a 64-bit 'tmp' variable
	 * bits 32-64 belong to the next integer (indicated by tmp>>32)
	 */
	tmp  = hash_val[0] + ((uint64)hash_val[0]<<1) + ((uint64)hash_val[0]<<5)+((uint64)hash_val[0]<<6)+((uint64)hash_val[0]<<8);
	result[0] = tmp;
	tmp  = hash_val[1] + ((uint64)hash_val[1]<<1) + ((uint64)hash_val[1]<<5)+((uint64)hash_val[1]<<6)+((uint64)hash_val[1]<<8)+(tmp>>32);
	result[1] = tmp;
	tmp  = hash_val[2] + ((uint64)hash_val[2]<<1) + ((uint64)hash_val[2]<<5)+((uint64)hash_val[2]<<6)+((uint64)hash_val[2]<<8)+(tmp>>32);
	result[2] = tmp;
	tmp  = hash_val[3] + ((uint64)hash_val[3]<<1) + ((uint64)hash_val[3]<<5)+((uint64)hash_val[3]<<6)+((uint64)hash_val[3]<<8)+(tmp>>32);
	result[3] = tmp;
	tmp  = hash_val[4] + ((uint64)hash_val[4]<<1) + ((uint64)hash_val[4]<<5)+((uint64)hash_val[4]<<6)+((uint64)hash_val[4]<<8)+(tmp>>32);
	result[4] = tmp;
	tmp  = hash_val[5] + ((uint64)hash_val[5]<<1) + ((uint64)hash_val[5]<<5)+((uint64)hash_val[5]<<6)+((uint64)hash_val[5]<<8)+(tmp>>32)+((uint64)hash_val[0]<<8);
	result[5] = tmp;
	tmp  = hash_val[6] + ((uint64)hash_val[6]<<1) + ((uint64)hash_val[6]<<5)+((uint64)hash_val[6]<<6)+((uint64)hash_val[6]<<8)+(tmp>>32)+((uint64)hash_val[1]<<8);
	result[6] = tmp;
	result[7]  = hash_val[7] + ((uint64)hash_val[7]<<1) + ((uint64)hash_val[7]<<5)+((uint64)hash_val[7]<<6)+((uint64)hash_val[7]<<8)+(tmp>>32)+((uint64)hash_val[2]<<8);
}



/*
void fnv64Bit(char hashstring[], uint64 *hashv, int start, int end)
{
    //unsigned char *s = (unsigned char *)hashstring;
    while (start < end) {
        *hashv ^= (unsigned char)hashstring[start++];
        *(hashv) += (*(hashv) << 1) + (*(hashv) << 4) + (*(hashv) << 5) +
                      (*(hashv) << 7) + (*(hashv) << 8) + (*(hashv) << 40);
    }
}*/
