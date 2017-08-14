/* 
 * File:   bloom.h
 * Author: Frank Breitinger
 *
 * Created on 17. April 2013, 23:34
 */

#ifndef FINGERPRINT_H
#define	FINGERPRINT_H

#include "bloomfilter.h"

#define BLOOMFILTER_SIZE_FILE 256 // 256 bytes is the default for a mrsh_v2 fingerprint

typedef struct FINGERPRINT {
	//List of Bloom filters
    BLOOMFILTER *bf_list;
    BLOOMFILTER *bf_list_last_element;
    
    //Pointer to next fingerprint
    struct FINGERPRINT *next;
    
   // After storing of MAXBLOCKS blocks are inserted, a new filter is created.
   // 'amount_of_BF' counts the number of filters we have for a file
   unsigned int  amount_of_BF;

   // File name and size of the original file
   char          file_name[200];
   unsigned int  filesize;
        
} FINGERPRINT;


FINGERPRINT     *init_empty_fingerprint();

int             destroy_fingerprint(FINGERPRINT *);

int             fingerprint_compare(FINGERPRINT *, FINGERPRINT *);

int             bloom_max_score(BLOOMFILTER *, FINGERPRINT *);

void            add_hash_to_fingerprint(FINGERPRINT *, uint256);

void            add_new_bloomfilter(FINGERPRINT *, BLOOMFILTER *);

double          compute_e_min(int, int);

//unsigned int        read_input_hash_file(FINGERPRINT_LIST *fpl,FILE *handle);

void            print_fingerprint(FINGERPRINT *);


#endif	/* BLOOM_H */




