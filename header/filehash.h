//
// Created by David Lillis on 23/09/2016.
//

#include "config.h"
#include "helper.h"
#include "fingerprint.h"

#ifndef FILE_HASH_H

#define FILE_HASH_H

typedef struct HASH_ENTRY {
    uint256 *value;
    int     position;
    struct HASH_ENTRY *next_entry;
} HASH_ENTRY;

typedef struct {
    HASH_ENTRY          *first_hash;
    HASH_ENTRY          *last_hash;
    int                 size; // number of hash entries
    int                 filesize;
    char                *filename;
    FINGERPRINT         *fingerprint;
} FILE_HASH;

FINGERPRINT     *init_fingerprint_for_file(FILE_HASH *);

void        destroy_hash_entry(HASH_ENTRY *);

void        destroy_file_hash(FILE_HASH *);

FILE_HASH   *init_file_hash();

HASH_ENTRY  *init_hash_entry(uint256 *, int);

void        add_hash_entry(FILE_HASH *, HASH_ENTRY *);
#endif //MRSH_NET_EXTENDED_FILE_HASH_H
void print_hash_entry( HASH_ENTRY * );
void print_file_hash( FILE_HASH * );
