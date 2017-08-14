#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../header/config.h"
#include "../header/hashing.h"
#include "../header/bloomfilter.h"

//static int SHIFTOPS;       	//FILTER_AS_POW_2 + 3 -> 14
//static unsigned long MASK;			//SHIFTOPS in ones    -> 0x1FFFF

//void initialize_settings(){
////	SHIFTOPS = (int)log2(BF_SIZE_IN_BYTES) + 3; // why?
//    SHIFTOPS = (int)log2(BF_SIZE_IN_BYTES)+3;
//	MASK = 0xFFFFFFFFFFFFFFFF >> (64-SHIFTOPS);
//}


//Returns an empty Bloom filter
BLOOMFILTER *init_empty_BF(unsigned long size) {
    BLOOMFILTER *bf;

    // allocate memory for the BF itself
    if (!(bf = (BLOOMFILTER *) calloc(1, sizeof(BLOOMFILTER)))) {
        fprintf(stderr, "[*] Error in initializing bloom filter.\n");
        exit(-1);
    }

    // allocate (and zero-initialise) BF's array
    if (!(bf->array = calloc(size, sizeof(char)))) {
        fprintf(stderr, "[*] Error in initializing bloom filter.\n");
        exit(-1);
    }

    bf->shiftops = (int) log2(size) + 3; // TODO: why the + 3?

    bf->mask = 0xFFFFFFFFFFFFFFFF >> (64 - bf->shiftops);
    bf->size = size;
    bf->amount_of_blocks = 0;
    return bf;
}


//Destroy a Bloom filter
void destroy_bf(BLOOMFILTER *bf) {
    free(bf->array);
    free(bf);
}


/*
 * adds a hash value (eg. FNV) to the Bloom filter
 */
void add_hash_to_bloomfilter(BLOOMFILTER *bf, uint256 hash_val) {
//    printf("ADDR: %p\n", bf);
//    printf("INS: ");
//    unsigned int* v = hash_val;
//    for( int i = 0; i < 8; i++) {
//        printf("%08x", *(v++));
//    }
//    printf("\n");

    uint64 masked_bits, byte_pos;
    short bit_pos;
    unsigned char *test = hash_val;

    uint64 *p = hash_val;
    uint64 tmpHash = (((uint64) hash_val[1] << 32) ^ hash_val[0]);

    //add the hash value to the bloom filter
    for (int j = 0; j < SUBHASHES; j++) {

        //get least significant bytes and use one relevant by AND MASK
        masked_bits = tmpHash & bf->mask;

//        printf("INS: %d %llu %lx %llu\n", j, masked_bits, bf->mask, bf->size);

        //get byte and bit position
        byte_pos = masked_bits >> 3;
        bit_pos = masked_bits & 0x7;

        //Set bit in BF
        bf->array[byte_pos] |= (1 << (bit_pos));

        //shift and continue
        p = &test[bf->shiftops * (j + 1) / 8];
        tmpHash = *p >> ((bf->shiftops * (j + 1)) % 8);
    }

    //count blocks / elements that are inserted
    bf->amount_of_blocks++;
}


short is_in_bloom(BLOOMFILTER *bf, uint256 hash_val) {
//    printf("ADDR: %p\n", bf);
//    printf("SRC: ");
//    unsigned int* v = hash_val;
//    for( int i = 0; i < 8; i++) {
//        printf("%08x", *(v++));
//    }
//    printf("\n");

    uint64 masked_bits, byte_pos;
    short bit_pos;
    unsigned char *test = hash_val;

    uint64 *p = hash_val;
    uint64 tmpHash = (((uint64) hash_val[1] << 32) ^ hash_val[0]);

    for (int j = 0; j < SUBHASHES; j++) {
        //get least significant bytes and use one relevant by AND MASK
        masked_bits = tmpHash & bf->mask;

//        printf("SRC: %d %llu %lx %llu\n", j, masked_bits, bf->mask, bf->size);

        //get byte and bit position
        byte_pos = masked_bits >> 3;
        bit_pos = masked_bits & 0x7;

        //if position in BF is zero then element isn't in BF
        if (((bf->array[byte_pos] >> bit_pos) & 1) != 1)
            return 0;

        //shift and continue
        p = &test[bf->shiftops * (j + 1) / 8];
        tmpHash = (*p) >> ((bf->shiftops * (j + 1)) % 8);
    }
    return 1;
}





/*short is_in_bloom(BLOOMFILTER *bf, unsigned int hashvalue){
	unsigned short masked_bits;
	short byte_pos,bit_pos, one_counter=0;
	unsigned int subHash = 0;

	for(int j=0;j<SUBHASHES;j++) {
	    masked_bits = ( hashvalue >> (SHIFTOPS * j)) & MASK;
        byte_pos = masked_bits >> 3;
        bit_pos = masked_bits & 0x7;

        if(((bf->array[byte_pos]>>bit_pos) & 0x1) != 1)
        	return 0;
	}
	return 1;
}*/


/*
 * Convert a hex string to a binary sequence (used for reading in hash lists)
 */
void convert_hex_binary(const unsigned char *hex_string, BLOOMFILTER *bf) {
    unsigned int i = 0;

    //WARNING: no sanitization or error-checking whatsoever
    for (i = 0; i < bf->size; i++) {
        sscanf(hex_string, "%2hhx", &bf->array[i]);
        hex_string += 2 * sizeof(char);
    }
}

void print_bf(BLOOMFILTER *bf) {
    int j;
    //BLOOMFILTER *bf = fp->bf_list;

    /* FORMAT: filename:filesize:number of filters:blocks in last filter*/
    //printf("%s:%d:%d:%d", fp->file_name, fp->filesize, fp->amount_of_BF, fp->bf_list_last_element->amount_of_blocks);
    //printf(":");

    // printf("%i:", bf->amount_of_blocks);



    //while(bf != NULL) {
    //Print each Bloom filter as a 2-digit-hex value
    for (j = 0; j < bf->size; j++)
        printf("%c", bf->array[j]);

    //move to next Bloom filter
    // bf = bf->next;
    // }
    //  printf("\n\n");

}

void pretty_print_bf(BLOOMFILTER *bf) {
    for (int i = 0; i < bf->size; i++) {
        printf("%d", bf->array[i]);
    }
}


void readFileToBF(const char *filename, BLOOMFILTER *bf) {
    int i;
    FILE *fp = fopen(filename, "rb");
    if (fp != 0) {
        for (i = 0; fread(&bf[i], sizeof(bf[i]), 1, fp) == 1; i++);
        fclose(fp);
    }
}

BLOOMFILTER *bf_union(BLOOMFILTER *one, BLOOMFILTER *two) {
    if (one->size != two->size) {
        fprintf(stderr, "Cannot perform union on bloom filters of different sizes.");
        exit(1);
    }
    BLOOMFILTER *ret = init_empty_BF(one->size);
    for (long i = 0; i < ret->size; i++) {
        ret->array[i] = one->array[i] | two->array[i];
    }
    ret->number_of_files = one->number_of_files + two->number_of_files;
    ret->bytes = one->bytes + two->bytes;
    return ret;
}

short count_char_ones(unsigned char c) {
    short count = 0;
    while (c != 0) {
        count += c & 1;
        c >>= 1;
    }
    return count;
}

unsigned long count_ones(BLOOMFILTER *bf) {
    unsigned long count = 0;
    for (long i = 0; i < bf->size; i++)
        count += count_char_ones(bf->array[i]);
    return count;
}

float fullness(BLOOMFILTER *bf) {
    return count_ones(bf) / (float) (bf->size * 8);
}


