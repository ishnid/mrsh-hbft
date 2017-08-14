/**
 * AUTHOR: Frank Breitinger
 * DATE: April 2013
 * Email: Frank.Breitinger@cased.de
 */
#include <stdio.h>
#include <string.h> 			//is important for strtok!!

#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "../header/config.h"
#include "../header/fingerprintList.h"
#include "../header/helper.h"



/**
 * Initializes an empty Fingerprint
 */
FINGERPRINT_LIST *init_empty_fingerprintList(){
	FINGERPRINT_LIST *fpl;

	if (!(fpl=(FINGERPRINT_LIST *)malloc(sizeof(FINGERPRINT_LIST)))) {
        fprintf(stderr,"[*] Error in initializing bloom_read \n");
        exit(-1);
    }
	fpl->list = NULL;
	fpl->last_element = NULL;
    fpl->size   = 0;
    return fpl;
}


FINGERPRINT_LIST *init_fingerprintList_for_ListFile(char *filename){
	FINGERPRINT_LIST *fpl = init_empty_fingerprintList();
	FILE *file = getFileHandle(filename);
	read_fingerprint_file(fpl, file);
	return fpl;
}



/*
 * Destroys all filters and sets all memory free
 */
void destroy_fingerprint_list(FINGERPRINT_LIST *fpl) {
	FINGERPRINT *tmp, *node = fpl->list;	//start at the head.
    while(node != NULL){					//traverse entire list.
    	tmp = node;							//save node pointer
    	node = node->next;					//advance to next.
    	destroy_fingerprint(tmp);			//free the saved one.
    }

    fpl->list = NULL;						//finally, mark as empty list.
    fpl->last_element = NULL;

	free(fpl);
}


/*
 * Adds a new, last Fingerprint to the list
 */
void add_new_fingerprint(FINGERPRINT_LIST *fpl, FINGERPRINT *fp){
	if(fpl->list == NULL){
		fpl->list = fp;
		fpl->last_element = fp;
	} else {
		fpl->last_element->next = fp;
		fpl->last_element = fp;
	}
		fpl->size++;
}


/*
 * Does an all-against-all comparison of the list
 * but does not compare the file with itself.
 */
void all_against_all_comparsion(FINGERPRINT_LIST *fpl){
   int score;
   FINGERPRINT *tmp2, *tmp1 = fpl->list;

   while(tmp1 != NULL){
	   FINGERPRINT *tmp2 = tmp1->next;
	   while(tmp2 != NULL){
		    score=fingerprint_compare(tmp1, tmp2);
	         if(score >= THRESHOLD)
	               printf("%s | %s | %.3i \n", tmp1->file_name, tmp2->file_name, score);
	         tmp2=tmp2->next;

	   }
	   tmp1=tmp1->next;
   }
}


/*
 * Compares two lists of hashes with each other.
 */
void fingerprint_list_comparsion(FINGERPRINT_LIST *fpl1, FINGERPRINT_LIST *fpl2){
   int score;
   FINGERPRINT *tmp1 = fpl1->list;

   while(tmp1 != NULL){
	   FINGERPRINT *tmp2 = fpl2->list;
	   while(tmp2 != NULL){
		    score=fingerprint_compare(tmp1, tmp2);
	         if(score >= THRESHOLD)
	               printf("%s | %s | %.3i \n", tmp1->file_name, tmp2->file_name, score);
	         tmp2=tmp2->next;
	   }
	   tmp1=tmp1->next;
   }
}

/*
 * Compares a fingerprint against a list of hashes
 */
void fingerprint_against_list_comparison(FINGERPRINT_LIST *fpl, FINGERPRINT *fp){

   int score;
   FINGERPRINT *tmp1 = fpl->list;

   while(tmp1 != NULL){
	     score=fingerprint_compare(tmp1, fp);
	         if(score >= THRESHOLD)
	               printf("%s | %s | %.3i \n", tmp1->file_name, fp->file_name, score);
	   tmp1=tmp1->next;
   }
}


void print_fingerprintList(FINGERPRINT_LIST *fpl){
	FINGERPRINT *tmp1 = fpl->list;

	while(tmp1 != NULL){
		print_fingerprint(tmp1);
		tmp1=tmp1->next;
	}
}


/*
 * Reads a fingerprint file and stores it in the fingerprint list
 */
unsigned int read_fingerprint_file(FINGERPRINT_LIST *fpl, FILE *handle){
	unsigned int bytes_read;
    size_t nbytes = 5000;
    unsigned char *hex_string, *tokenize, *string_read;   	//the hex string of the hash
    char delims[] = ":"; 									//separator for the fingerprints in the file
    int amount_of_BF=0, blocks_in_last_bf=0;


    //hex needs to be doubled because 2 characters is one hex value
    unsigned char *hex = (unsigned char *) malloc(BLOOMFILTER_SIZE_FILE*2 +1);

    /*Main lines for a glibc getline function*/
    /*getline is only for *nix machines. uses glibc. more reliable and reallocates the memory of the buffer while reading*/
    while(!feof(handle)){
    	string_read =( unsigned char *) malloc(nbytes +1);
        bytes_read = getline(&string_read, &nbytes, handle);


        //error handling
        if(bytes_read == -1) {
            free(string_read);
            break;
        } else if(!strcmp(string_read, " ") || !strcmp(string_read,"\n")) {
            continue;
        } else {
           //parse the string read and extract the hashed hexadecimal string
            /*strtok is used for tokenizing the string (separation delims)*/
            FINGERPRINT *fp = init_empty_fingerprint();
            add_new_fingerprint(fpl, fp);

            tokenize = strtok(string_read,delims);

            int counter = 0;
            while(tokenize !=NULL){
                switch(counter) {
                    case 0:
                        /*get the filename*/
                        strcpy(fp->file_name, tokenize); break;

                    case 1:
                        /*get the filesize*/
                        fp->filesize = atoi(tokenize); break;

                    case 2:
                        /*get the count of the filters*/
                        amount_of_BF= atoi(tokenize); break;

                    case 3:
                        /* only the last block of the filter have less than max blocks*/
                    	blocks_in_last_bf = atoi(tokenize); break;

                    case 4:
                    	/* hex_string is then the fingerprint */
                        hex_string =(unsigned char*) calloc((strlen(tokenize)+1), sizeof(unsigned char));
                        strcpy(hex_string, tokenize);
                        break;

                    default:
                        fprintf(stderr, "[*] ERROR IN PARSING FILE CONTENT OF HASH FILE");
                        break;
                }
                tokenize= strtok(NULL, delims);
                counter++;
            }


           if(hex_string!=NULL){

        	   //Reset bf_list when we read in a LIST
        	   fp->bf_list = NULL;
        	   fp->bf_list_last_element = NULL;

        	   for(int i=0; i<=amount_of_BF;i++){
        		   //create a Bloom filter and add it to the fingerprint
        		   BLOOMFILTER *bf = init_empty_BF(BLOOMFILTER_SIZE_FILE);
        		   add_new_bloomfilter(fp, bf);

        		   //fill Bloom filter with the hex digest
        		   //example: void * memcpy ( void * destination, const void * source, size_t num );
        		   memcpy(hex, &hex_string[BLOOMFILTER_SIZE_FILE*2*i], BLOOMFILTER_SIZE_FILE*2);
        		   convert_hex_binary(hex, bf);

        		   bf->amount_of_blocks = MAXBLOCKS;
       		    }

        	   //The last Bloom filter may not have MAXBLOCKS --> update it
        	   fp->bf_list_last_element->amount_of_blocks = blocks_in_last_bf;

        	   free(hex_string);
        	   hex_string=NULL;
            }


           if(string_read!=NULL){
                free(string_read);
                string_read=NULL;
            }
    }
    }
	fclose(handle);
    return 1;
}


