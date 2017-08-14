//
// Created by David Lillis on 08/09/2016.
//

#include "helper.h"
#include "bloomfilter.h"
#include "fingerprintList.h"

enum tree_type {FIXED, VARIABLE};

typedef struct {
    BLOOMFILTER **data;         // array of pointers to BLOOMFILTERs (BFs)
    int size;                   // number of BFs in tree
    bool built;                 // indicate whether tree has been fully built (if bottom-up construction, this will initially be false)
    enum tree_type type;        // fixed-width BFs or variable based on depth?
    int next_insert;            // track which leaf will be inserted into next (assumes round-robin insertion)
    FINGERPRINT_LIST **fpls;    // leaf nodes record a list of fingerprints for the file in this node
} BLOOMFILTER_TREE;

BLOOMFILTER_TREE    *init_fixed_bf_tree(unsigned int, unsigned long);        // initialise a BLOOMFILTER_TREE (BFT) with fixed-width BFs
BLOOMFILTER_TREE    *init_variable_bf_tree(unsigned int, unsigned long);     // initialise a BFT with variable-width BFs
int                 *search(BLOOMFILTER_TREE*, FILE_CONTENTS*);

int                 leaves(BLOOMFILTER_TREE *);
BLOOMFILTER         *get_leaf_bf( BLOOMFILTER_TREE*, unsigned int );
void                add_path_to_bf_tree(BLOOMFILTER_TREE *, char *);
void                search_path_in_bf_tree(BLOOMFILTER_TREE *, char *);
void                save_bf_tree( char *filename, BLOOMFILTER_TREE *);

void                build_bf_tree(BLOOMFILTER_TREE *);

void                hash_file_to_bf( BLOOMFILTER_TREE*, int, FILE_CONTENTS* );

void                destroy_bf_tree(BLOOMFILTER_TREE *);

// returns index of the leaf it was inserted into
int                 insert_into_bf_tree(BLOOMFILTER_TREE *, FILE_CONTENTS *);

BLOOMFILTER_TREE    *load_bf_tree( char * );