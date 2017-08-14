//
// Created by David Lillis on 23/09/2016.
//

#include <stdlib.h>
#include <memory.h>
#include "../header/config.h"
#include "../header/filehash.h"

void print_hash_entry(HASH_ENTRY *he) {
    unsigned int *value = he->value;
    for (int i = 0; i < 8; i++) {
        printf("%08x", value[i]);
    }
    printf("\n");
}

void print_file_hash(FILE_HASH *fh) {
    printf("-------------- FILE HASH --------------\n");

    HASH_ENTRY *he = fh->first_hash;
    for (int i = 0; i < fh->size; i++) {
        print_hash_entry(he);
        he = he->next_entry;
    }
    printf("------------ END FILE HASH ------------\n");
}

void destroy_hash_entry(HASH_ENTRY *he) {
    free(he->value);
    free(he);
}

void destroy_file_hash(FILE_HASH *fh) {
    HASH_ENTRY *he = fh->first_hash;
    HASH_ENTRY *next;
    while (he != NULL) {
        next = he->next_entry;
        destroy_hash_entry(he);
        he = next;
    }
    if (fh->fingerprint != NULL)
        destroy_fingerprint(fh->fingerprint);
}

void hash_file_to_fingerprint(FINGERPRINT *fp, FILE_HASH *fh) {
    BLOOMFILTER *last_bf = fp->bf_list_last_element;
    HASH_ENTRY *he = fh->first_hash;
    while (he != NULL) {

        if (last_bf->amount_of_blocks == MAXBLOCKS) {
            BLOOMFILTER *new_bf = init_empty_BF(BLOOMFILTER_SIZE_FILE);
            add_new_bloomfilter(fp, new_bf);
            last_bf = new_bf;
        }

        add_hash_to_bloomfilter(last_bf, he->value);

        he = he->next_entry;
    }
}

/**
 *
 * Creates a Fingerprint for a File (sets filename and filezite)
 * returns NULL if it is not able to create the bloom filter,
 * else the allocated FINGERPRINT struct
 * */
FINGERPRINT *init_fingerprint_for_file(FILE_HASH *fh) {
    FINGERPRINT *fp = init_empty_fingerprint();
    fp->filesize = fh->filesize;
    strcpy(fp->file_name, fh->filename);
    hash_file_to_fingerprint(fp, fh);

    return fp;
}

FILE_HASH *init_file_hash() {
    FILE_HASH *fh = calloc(1, sizeof(FILE_HASH));
    if (fh == NULL) {
        fprintf(stderr, "Failed to initialise file hash\n");
        exit(1);
    }
    return fh;
}

HASH_ENTRY *init_hash_entry(uint256 *hash_value, int position) {
    HASH_ENTRY *he = calloc(1, sizeof(HASH_ENTRY));
    if (he == NULL) {
        fprintf(stderr, "Failed to initialised hash entry.\n");
        exit(1);
    }
    he->value = hash_value;
    he->position = position;

    return he;
}

void add_hash_entry(FILE_HASH *fh, HASH_ENTRY *he) {
    if (fh->size == 0)
        fh->first_hash = he;
    else
        fh->last_hash->next_entry = he;
    fh->last_hash = he;
    fh->size++;
}

