/* 
 * File:   bloom.h
 *
 * Created on 17. April 2012, 23:34
 */

#include "fnv.h"

#ifndef BLOOMFILTER_H
#define	BLOOMFILTER_H

/* 
 * We define a struct BLOOM, with all the properties a BLoom-Filter needs.
 */
typedef struct {
    // Pointer to a byte array
    unsigned char *array;
    
    // We store the number of blocks we add to each filter in count_added_blocks
    unsigned int amount_of_blocks;

    // Record the number of bytes in this BF
    unsigned long size;

    int shiftops;

    unsigned long mask;

    // Pointer to next Bloomfilter (if used as a list)
    struct BLOOMFILTER *next_filter;

    unsigned long bytes; // number of bytes worth of files this filter represents

    unsigned int number_of_files; // number of files this filter represents
}BLOOMFILTER;


void 			initialize_settings();

BLOOMFILTER     *init_empty_BF(unsigned long);
void            destroy_bf(BLOOMFILTER *);

void            bloom_set_bit(unsigned char *, unsigned short);
short			is_in_bloom(BLOOMFILTER *, uint256 );

void 			add_hash_to_bloomfilter(BLOOMFILTER *, uint256);
void            convert_hex_binary(const unsigned char *, BLOOMFILTER *);

void 			print_bf(BLOOMFILTER *);
void 			readFileToBF(const char *, BLOOMFILTER *);

// percentage of bits in the BF that are 1.
float             fullness( BLOOMFILTER * );

//count number of set bits in BF
unsigned long   count_ones(BLOOMFILTER* );
short           count_char_ones( unsigned char c );

#endif	/* BLOOM_H */

BLOOMFILTER * bf_union( BLOOMFILTER*, BLOOMFILTER*);

