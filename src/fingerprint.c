/**
 * AUTHOR: Frank Breitinger
 * DATE: April 2013
 * Email: Frank.Breitinger@cased.de
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../header/config.h"
#include "../header/fingerprint.h"
#include "../header/main.h"
#include "../header/bloomfilter.h"
#include "../header/filehash.h"

/**
 * Initializes an empty Fingerprint
 */
FINGERPRINT *init_empty_fingerprint(){
	FINGERPRINT *fp;

	if (!(fp=(FINGERPRINT *)malloc(sizeof(FINGERPRINT)))) {
        fprintf(stderr,"[*] Error in initializing bloom_read \n");
        exit(-1);
    }

    fp->amount_of_BF       = 0;
    fp->bf_list = init_empty_BF( BLOOMFILTER_SIZE_FILE );
    fp->bf_list_last_element = fp->bf_list;
    fp->next = NULL;
    return fp;
}



// will only work for BLOOMFILTER_SIZE_FILE sized filters
unsigned short bloom_common_bits(BLOOMFILTER *bf1, BLOOMFILTER *bf2) {
    short ret = 0;
    for(int a=0;a<BLOOMFILTER_SIZE_FILE;a++)
        ret += count_char_ones( bf1->array[a] & bf2->array[a] );
    return ret;
}





/*
 *  Destroys all filters and sets all memory free
 */
int destroy_fingerprint(FINGERPRINT *fp) {
	BLOOMFILTER *tmp, *node = fp->bf_list;	//start at the head.
    while(node != NULL){					//traverse entire list.
    	tmp = node;							//save node pointer
    	node = node->next_filter;					//advance to next.
    	free(tmp);							//free the saved one.
    }
    fp->bf_list = NULL;						//finally, mark as empty list.
    fp->bf_list_last_element = NULL;

	free(fp);
    fp=NULL;
    return 0;
}


/*
 * Adds a block-hash to the fingerprint
 */
void add_hash_to_fingerprint(FINGERPRINT *fp, uint256 hash_value){
	BLOOMFILTER *lastBF = fp->bf_list_last_element;

	//if MAXBLOCKS are within a Bloom filter than create a new one
	if(lastBF->amount_of_blocks == MAXBLOCKS){
		BLOOMFILTER *bf = init_empty_BF(BLOOMFILTER_SIZE_FILE);
		add_new_bloomfilter(fp, bf);
		lastBF = bf;
	}

	add_hash_to_bloomfilter(lastBF, hash_value);
}


//Adds a new last Bloom filter
void add_new_bloomfilter(FINGERPRINT *fp, BLOOMFILTER *bf){
	if(fp->bf_list==NULL) {
		fp->bf_list = bf;
		fp->bf_list_last_element = bf;
	} else {
		fp->bf_list_last_element->next_filter = bf;
		fp->bf_list_last_element = bf;
		fp->amount_of_BF++;
	}
}


// Compares two fingerprints and returns a match-score between 0 and 100
int fingerprint_compare(FINGERPRINT *fingerprint1, FINGERPRINT *fingerprint2) {

    int final_score = 0;
    int i, amount_of_BF;

    FINGERPRINT *larger_fingerprint = fingerprint1;
    FINGERPRINT *smaller_fingerprint = fingerprint2;

    //Smaller fingerprint needs to be identified to generate the correct match score
    if(fingerprint1->amount_of_BF < fingerprint2->amount_of_BF) {
        larger_fingerprint = fingerprint2;
        smaller_fingerprint = fingerprint1;
    }

   	amount_of_BF = smaller_fingerprint->amount_of_BF+1;
   	if(smaller_fingerprint->bf_list_last_element->amount_of_blocks < MINBLOCKS)
		amount_of_BF--;

    //run through all bloom filters of the smaller fingerprint and compare them
    //to all fingerprints of the large one
    BLOOMFILTER *bf = smaller_fingerprint->bf_list;



    for(i=0;i<=smaller_fingerprint->amount_of_BF && bf->amount_of_blocks >= MINBLOCKS;i++) {
    	final_score += bloom_max_score(bf, larger_fingerprint);        //final_score += bloom_max_score(filter_ptr[i], bloom_less_elements->count_added_blocks[i], bloom_more_elements);
    	bf = bf->next_filter;
    }

    //generate the score
    if(amount_of_BF < 1)
    	return 0;

    return final_score/amount_of_BF;
}




int bloom_max_score(BLOOMFILTER *bf, FINGERPRINT *fingerprint) {
    int    C, i, e_min, e_max;
    int tmp_score = 0;
    int score     = 0;

    int bitsSetOfBF1 = count_ones(bf);

    BLOOMFILTER *tmp_bf = fingerprint->bf_list;

    e_min = compute_e_min(bf->amount_of_blocks, tmp_bf->amount_of_blocks);

    for(i=0;i<=fingerprint->amount_of_BF;i++) {

    	//Filters with 6 or less elements are critical
    	if(tmp_bf->amount_of_blocks < MINBLOCKS){
    		//if(i > 0 || (mode->ignoreSmallInputs)) //&& bf->amount_of_blocks > 40))
    				return score;
    	}

    	//for the last Bloom filter we have to update the values
       	if(tmp_bf->next_filter == NULL) {
           	e_min = compute_e_min(tmp_bf->amount_of_blocks, bf->amount_of_blocks);
        }

       	e_max = MIN(bitsSetOfBF1, count_ones(tmp_bf));
   	    C = 0.3*(e_max - e_min)+e_min;




       	//compute bits in common
        unsigned int numofbitsInCommon = bloom_common_bits(tmp_bf, bf);

      	 if(numofbitsInCommon > C)
      		 printf("");

        //if they are high enough we have a threshold
        if(numofbitsInCommon < C) {
            tmp_score = 0;
        } else {
        	if ((e_max - C) >= 1)
        		tmp_score = 100*(numofbitsInCommon-C)/(e_max-C);
        }

        if(score < tmp_score){
            score = tmp_score;
            if(score == 100)
            	break;
        }

        tmp_bf = tmp_bf->next_filter;
    }
    return score;
}


double compute_e_min(int blocks_in_bf1, int blocks_in_bf2){
	int b1 = blocks_in_bf1;
	int b2 = blocks_in_bf2;

	double tmp1 = pow(PROBABILITY, SUBHASHES*b1);
	double tmp2 = pow(PROBABILITY, SUBHASHES*b2);
	double tmp3 = pow(PROBABILITY, SUBHASHES*(b1+b2));

	return BLOOMFILTER_SIZE_FILE * 8 * (1 - tmp1 - tmp2 + tmp3);
	//return BLOOMFILTERBITSIZE * (1-pow(bloom->probability,5*blocks)-pow(bloom->probability,5*blocks)+pow(tmp_bf->probability,5*(blocks+tmp_bf->count_added_blocks[i])))
}



void print_fingerprint(FINGERPRINT *fp){
    int j;
    BLOOMFILTER *bf = fp->bf_list;

    /* FORMAT: filename:filesize:number of filters:blocks in last filter*/
    printf("%s:%d:%d:%d", fp->file_name, fp->filesize, fp->amount_of_BF, fp->bf_list_last_element->amount_of_blocks);
    printf(":");

    while(bf != NULL) {
    	//Print each Bloom filter as a 2-digit-hex value
    	for(j=0;j<bf->size;j++)
        	printf("%02X", bf->array[j]);

       //move to next Bloom filter
       bf = bf->next_filter;
    }
    printf("\n\n");

}