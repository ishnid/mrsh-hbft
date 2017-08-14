#include "../header/hashing.h"
#include "../header/config.h"
#include "../header/fnv.h"
#include "../header/helper.h"
#include "../header/bloomfilter.h"
#include "../header/filehash.h"
#include <stdio.h>

#include <math.h>



uint32 roll_hashx(unsigned char c, uchar window[], uint32 rhData[])
{
    uint32 t = rhData[0] % ROLLING_WINDOW;

	rhData[2] -= rhData[1];
    rhData[2] += (ROLLING_WINDOW * c);

    rhData[1] += c;
    rhData[1] -= window[t];

    window[t] = c;
    rhData[0]++;

    /*
    rhData[1] += c;
       rhData[1] -= window[rhData[0]];

       window[rhData[0]] = c;
       rhData[0] = (++rhData[0]) % ROLLING_WINDOW;*/
    /* The original spamsum AND'ed this value with 0xFFFFFFFF which
       in theory should have no effect. This AND has been removed 
       for performance (jk) */
    rhData[3] = (rhData[3] << 5); //& 0xFFFFFFFF;
    rhData[3] ^= c;

    return rhData[1] + rhData[2] + rhData[3];
}



int hashFileToBF(BLOOMFILTER *bf, FILE *handle){
	hashAndDo(bf, handle, 1);
	return 1;
}


int *hashFileAndCompare(BLOOMFILTER *bf, FILE *handle) {
	return hashAndDo(bf, handle, 0);
}


FILE_HASH *hash_file( FILE_CONTENTS *fc ) {

    FILE_HASH *fh = init_file_hash();

    unsigned int   i;

    unsigned int last_block_index = 0;

    uint32 rValue;
    unsigned int *hash_val;
    bool is_first = true;

    /*we need this arrays for our extended rollhash function*/
    uchar window[ROLLING_WINDOW] = {0};
    uint32 rhData[4]             = {0};

    int position = 0;

    //run through all bytes
    for(i=0; i<fc->filesize; i++){

        //build rolling hash
        rValue = roll_hashx(fc->data[i], window, rhData);

        //check for end of chunk
        if (rValue % mode->block_size == mode->block_size -1) {
            //we may skip the first chunk (eg. this is often header info)
            if (is_first && SKIP_FIRST) {
                is_first = false;
            }
            else {
                //hash the chunk and add hash to bloom filter
                hash_val = fnv256Bit(fc->data, last_block_index, i);
                add_hash_entry(fh, init_hash_entry(hash_val, position++));

                last_block_index = i + 1;
                if (i + SKIPPED_BYTES <
                    fc->filesize) // Dave: fixed - original code used uninitialised bytes_read variable
                    i += SKIPPED_BYTES;
            }
        }
    }

    fh->filesize = fc->filesize;
    fh->filename = malloc( (1 + strlen( fc->filename)) * sizeof(char));
    if ( fh->filename == NULL ) {
        fprintf( stderr, "Failed to allocate file name space.");
        exit(1);
    }
    strcpy(fh->filename, fc->filename);

//    print_file_hash(fh);

    return fh;
}

void add_file_hash_to_bf(BLOOMFILTER *bf, FILE_HASH *fh) {
    HASH_ENTRY *he = fh->first_hash;
    while ( he != NULL ) {
        add_hash_to_bloomfilter(bf, he->value);
        he = he->next_entry;
    }
    bf->number_of_files++;
    bf->bytes += fh->filesize;
}

//Depending on the third parameter it fills the Bloom filter (1) or it compares File against Bloom filter
int *hashAndDo(BLOOMFILTER *bf, FILE_CONTENTS *fc, short fill){

    unsigned int   i;

    unsigned int last_block_index = 0;

    int *results_summary = calloc(4, sizeof(int));

    uint32 rValue;
    unsigned int *hash_val;
    bool is_first = true;

    /*we need this arrays for our extended rollhash function*/
    uchar window[ROLLING_WINDOW] = {0};
    uint32 rhData[4]             = {0};

    int inserts = 0; //TODO: temporary

    //run through all bytes
    for(i=0; i<fc->filesize; i++){

    	//build rolling hash
    	rValue  = roll_hashx(fc->data[i], window, rhData);

    	//check for end of chunk
        if (rValue % mode->block_size == mode->block_size-1) {

        	//we may skip the first chunk (eg. this is often header info)
        	if(is_first && SKIP_FIRST){
        		is_first=false;
        	}
            else {

                //hash the chunk and add hash to bloom filter
                hash_val = fnv256Bit(fc->data, last_block_index, i);

                //check whether we are in comparison mode or checking mode
                if (fill == 1) {
                    add_hash_to_bloomfilter(bf, hash_val);
                    inserts++;
                } else
                    createResultsSummary(bf, hash_val, results_summary);

                free(hash_val); // TODO: this is temporary: perhaps better to alloc one hash val and pass it in as an argument

                last_block_index = i + 1;
                if (i + SKIPPED_BYTES < fc->filesize) // Dave: fixed - original code used uninitialised bytes_read variable
                    i += SKIPPED_BYTES;
            }
        }
    }

    //handling of the last block (last trigger point to EOF)
    if(!SKIP_LAST){
    	hash_val = fnv256Bit(fc->data, last_block_index, i);
    	if (fill==1)
    		add_hash_to_bloomfilter(bf, hash_val);
    	else
      		createResultsSummary(bf, hash_val, results_summary);
        free(hash_val);
    }

//    printf("inserts %d (from size %d)\n", inserts , fc->filesize);

    return results_summary;
}

int *hashPacketBuffer(BLOOMFILTER *bf, const unsigned char *packet, const size_t length)
{
    unsigned long  bytes_read;   //stores the number of characters read from input file
    unsigned int   i;
    unsigned int last_block_index = 0;
    double entropy_val = 0.0;
    int frequencyArray[256] = {0};
//    int results_summary[4] = {0}; 	//0 total blocks; 1 blocks found; 2 longest run; 3 tmp savings
    int *results_summary = calloc(4, sizeof(int));
    uint64 rValue;
    unsigned int *hash_val;
    short is_first = 1;

    /*we need this arrays for our extended rollhash function*/
    uchar window[ROLLING_WINDOW] = {0};
    uint32 rhData[4]             = {0};

    //run through all bytes
    for(i=0; i<length; i++){
    	//update frequency array for entropy
    	frequencyArray[packet[i]]++;

    	//build rolling hash
    	rValue  = roll_hashx(packet[i], window, rhData);

    	//check for end of chunk
        if (rValue % mode->block_size == mode->block_size-1) {

        	//we may skip the first chunk (eg. this is often header info)
        	if(is_first && SKIP_FIRST){
        		is_first=0;
        		continue;
        	}

        	//calculate entropy of piece
        	entropy_val = (double)entropy(frequencyArray, i-last_block_index);
        	if (entropy_val < MIN_ENTROPY)
        		continue;

        	//hash the chunk and add hash to bloom filter
        	hash_val = fnv256Bit(packet, last_block_index, i);

        	createResultsSummary(bf, hash_val, results_summary);

            last_block_index = i+1;
            if(i+SKIPPED_BYTES < bytes_read)
            	i += SKIPPED_BYTES;
        }
    }

    //handling of the last block (last trigger point to EOF)
    if(!SKIP_LAST){
    	hash_val = fnv256Bit(packet, last_block_index, i);
    	createResultsSummary(bf, hash_val, results_summary);
    }

    return results_summary;
}



double entropy(int freqArray[], int size) {
	double e = 0.0;
    double f = 0.0;
    int i =0;
    for(i=0; i<=255; i++) {
        if (freqArray[i] > 0) {
            f = (double) freqArray[i] / size;
            e -= f * log2(f);
            freqArray[i]=0;		//reset field
        }
    }
    return e;
}

void createResultsSummary(BLOOMFILTER *bf, uint256 hash_val, int *results_summary){
	results_summary[0]++;
	if(is_in_bloom(bf, hash_val)==1) {
		results_summary[1]++;					//counter for found chunks
	   	results_summary[3]++;
	   	if (results_summary[3] > results_summary[2]) 	//check if there is a long run
	   		results_summary[2] = results_summary[3];
	}else
		results_summary[3] = 0;
}

