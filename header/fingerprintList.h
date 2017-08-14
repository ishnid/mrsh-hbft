/* 
 * File:   bloom.h
 * Author: Frank Breitinger
 *
 * Created on 17. April 2013, 23:34
 */

#ifndef FINTERPRINTLIST_H
#define	FINTERPRINTLIST_H

#include "fingerprint.h"


typedef struct {
    //List of fingerprints, created on reading a file having many hashes
    FINGERPRINT *list;
    FINGERPRINT *last_element;
    
    // size of the list, used while freeing the memory
    unsigned int size;
    
}FINGERPRINT_LIST;


typedef struct{
    char    **filename;
    double  *score;
    long    length;
}MATCH_RESULT;


FINGERPRINT_LIST    *init_empty_fingerprintList();
FINGERPRINT_LIST 	*init_fingerprintList_for_ListFile(char *);
void                destroy_fingerprint_list(FINGERPRINT_LIST *);

void                add_new_fingerprint(FINGERPRINT_LIST *fpl, FINGERPRINT *fp);

void                all_against_all_comparsion(FINGERPRINT_LIST *);
void                fingerprint_list_comparsion(FINGERPRINT_LIST *, FINGERPRINT_LIST *);
void 				fingerprint_against_list_comparison(FINGERPRINT_LIST *, FINGERPRINT *);

void                print_fingerprintList(FINGERPRINT_LIST *);

unsigned int        read_fingerprint_file(FINGERPRINT_LIST *, FILE *);



#endif	/* BLOOM_H */




