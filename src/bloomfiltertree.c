//
// Created by David Lillis on 08/09/2016.
//

#include "../header/bloomfilter.h"
#include "../header/bloomfiltertree.h"
#include "../header/hashing.h"
#include "../header/filehash.h"
#include "../header/helper.h"
#include "../header/config.h"

#include <dirent.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

int left(int i) {
    return i * 2;
}

int right(int i) {
    return i * 2 + 1;
}

int parent(int i) {
    return i / 2;
}

bool is_leaf(BLOOMFILTER_TREE *tree, int i) {
    return left(i) > tree->size - 1;
}

bool is_root(int i) {
    return i == 1;
}

int depth(int i) {
    return (int) log2(i);
}

int leaves(BLOOMFILTER_TREE *bft) {
    return bft->size / 2;
}

void init_fingerprint_lists(BLOOMFILTER_TREE *bft) {
    bft->fpls = malloc(sizeof(FINGERPRINT_LIST *) * bft->size / 2);
    if (bft->fpls == NULL) {
        fprintf(stderr, "Failed to allocate memory for fingerprint list array.");
        exit(1);
    }

    for (int i = 0; i < bft->size / 2; i++) {
        bft->fpls[i] = init_empty_fingerprintList();
    }
}

/**
 * Initialise a bloom filter tree where the size of the BFs varies with depth.
 * @param leaves
 * @param root_bf_size_in_bytes
 * @return
 */
BLOOMFILTER_TREE *init_variable_bf_tree(unsigned int leaves, unsigned long root_bf_size_in_bytes) {

    BLOOMFILTER_TREE *ret;
    if ((ret = malloc(sizeof(BLOOMFILTER_TREE))) == NULL) {
        fprintf(stderr, "Failed to allocate memory for bloom filter tree.");
        exit(1);
    }
    ret->size = leaves * 2;
    ret->built = true;

    // define enough space to store parent nodes & leaves
    // NOTE: root node is @ index 1 for ease of calculation
    if ((ret->data = malloc(ret->size * sizeof(BLOOMFILTER *))) == NULL)
        printf("Data Allocation Failed\n");


    // add empty BFs in each position
    for (int i = 1; i < ret->size; i++)
        ret->data[i] = init_empty_BF(root_bf_size_in_bytes / (int) pow(2, depth(i)));

    ret->type = VARIABLE;
    ret->next_insert = 0;

#ifdef FINGERPRINT_LEAVES
    // generate the empty arrays necessary to store lists of fingerprints at the leaves
    init_fingerprint_lists(ret);
#endif

    return ret;
}

/**
 * Initialise a bloom filter tree where the size of the BFs is fixed.
 * @param leaves
 * @param bf_size_in_bytes
 * @return
 */
BLOOMFILTER_TREE *init_fixed_bf_tree(unsigned int leaves, unsigned long bf_size_in_bytes) {

    BLOOMFILTER_TREE *ret = malloc(sizeof(BLOOMFILTER_TREE));
    ret->size = leaves * 2;
    ret->built = false;

    // define enough space to store parent nodes & leaves
    // NOTE: root node is @ index 1 for ease of calculation
    if ((ret->data = malloc(ret->size * sizeof(BLOOMFILTER *))) == NULL)
        printf("Data Allocation Failed\n");


    // add leaf BFS in correct positions
    for (int i = 0; i < ret->size / 2; i++)
        ret->data[i + ret->size / 2] = init_empty_BF(bf_size_in_bytes);

    ret->type = FIXED;
    ret->next_insert = 0;

#ifdef FINGERPRINT_LEAVES
    // generate the empty arrays necessary to store lists of fingerprints at the leaves
    init_fingerprint_lists(ret);
#endif

    return ret;
}

void build_bf_tree(BLOOMFILTER_TREE *bft) {
    if (bft->type == FIXED && !bft->built) {
        // calculate remainder of the tree (fixed tree only)
        for (int i = bft->size - 1; i > 1; i -= 2)
            bft->data[(i - 1) / 2] = bf_union(bft->data[i - 1], bft->data[i]);
        bft->built = true;
    }
}

/*
 * Check if a file hash matches this bloom filter.
 *
 * (i.e. can we find consecutive matches up to a length of min_run?)
 */
bool match(BLOOMFILTER *bf, FILE_HASH *fh) {
    int hits = 0;
    bool is_first = true;
    HASH_ENTRY *he = fh->first_hash;

    while (he != NULL && hits < mode->min_run) {
        if (is_first && SKIP_FIRST) {
            is_first = false;
        } else {
            if (is_in_bloom(bf, he->value))
                hits++;
            else
                hits = 0;
            he = he->next_entry;
        }
    }
    return hits == mode->min_run;
}

/*
 * Recurisvely search a BF tree for a file hash, from a given node.
 *
 * Param bft is the tree.
 * Param fh is the file hash.
 * Param i is the index of the node to search.
 * Param result is for storing the result of the search.
 */
void find(BLOOMFILTER_TREE *bft, FILE_HASH *fh, int i, int *result) {
#ifdef LOGGING
    printf("Visit %d\n", i);
#endif

    if (match(bft->data[i], fh)) {
        if (is_leaf(bft, i)) {
#ifdef FINGERPRINT_LEAVES
            // fingerprint comparison (lazy loading)
            if (fh->fingerprint == NULL)
                fh->fingerprint = init_fingerprint_for_file(fh);
            // compare against all fingerprints in the leaf we have reached.
            fingerprint_against_list_comparison(bft->fpls[i - bft->size / 2], fh->fingerprint);
#endif

#ifdef LOGGING
            printf("Leaf %d\n", i - bft->size / 2);
#endif
            result[i - bft->size / 2] = true;
        } else {
            find(bft, fh, left(i), result);
            find(bft, fh, right(i), result);
        }
    }
}

int *search(BLOOMFILTER_TREE *bft, FILE_CONTENTS *fc) {

#ifdef LOGGING
    printf("----------------\nSearching for [%s]\n", fc->filename);
#endif

    // automatically build the tree if desired
    if (!bft->built && AUTO_BUILD_BF_TREE)
        build_bf_tree(bft);

    if (bft->type == VARIABLE || bft->built) {
        int *result = calloc(bft->size / 2, sizeof(int));
        FILE_HASH *fh = hash_file(fc);
        find(bft, fh, 1, result);
        destroy_file_hash(fh);
        return result;
    } else {
        fprintf(stderr, "Cannot search unbuilt Bloom Filter tree: you must call build() first.");
        exit(1);
    }
}

// get the BF at leaf i (0-indexed)
BLOOMFILTER *get_leaf_bf(BLOOMFILTER_TREE *bft, unsigned int i) {
    return bft->data[bft->size / 2 + i];
}

void print_file(FILE_CONTENTS *fc) {
    for (int i = 0; i < fc->filesize; i++) {
        printf("%c", fc->data[i]);
    }
    printf("\n");
}

void hash_file_to_bf(BLOOMFILTER_TREE *bft, int leaf, FILE_CONTENTS *fc) {

    FILE_HASH *fh = hash_file(fc);
    
    add_file_hash_to_bf(get_leaf_bf(bft, leaf), fh);

#ifdef FINGERPRINT_LEAVES
    add_new_fingerprint(bft->fpls[leaf], init_fingerprint_for_file(fh));
#endif

    // if this is a variable tree, also insert into
    if (bft->type == VARIABLE) {

        int current = leaf + bft->size / 2;

        while (!is_root(current)) {
            add_file_hash_to_bf(bft->data[parent(current)], fh);
            current = parent(current);
        }
    }

    destroy_file_hash(fh);
}

void destroy_bf_tree(BLOOMFILTER_TREE *bft) {

    // avoid freeing memory that was never allocated by building
    int i = bft->type == VARIABLE || bft->built ? 1 : leaves(bft);
    while (i < bft->size)
        destroy_bf(bft->data[i++]);
#ifdef FINGERPRINT_LEAVES
    for (i = 0; i < bft->size / 2; i++)
        destroy_fingerprint_list(bft->fpls[i]);
    free(bft->fpls);
#endif
    free(bft->data);
    free(bft);
}

int insert_into_bf_tree(BLOOMFILTER_TREE *bft, FILE_CONTENTS *fc) {
    int index = bft->next_insert;
    hash_file_to_bf(bft, index, fc);
    bft->next_insert = (bft->next_insert + 1) % leaves(bft);
    return index;
}


void save_bf_tree(char *filename, BLOOMFILTER_TREE *bft) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "Cannot write file %s\n", filename);
        exit(1);
    }
    build_bf_tree(bft); // make sure its been built
    fprintf(f, "%c%d:%d:", bft->type == VARIABLE ? 'c' : 'f', get_leaf_bf(bft, 0)->size, leaves(bft));
    // write each BF in turn
    for (int i = 1; i < bft->size; i++) {
        BLOOMFILTER *bf = bft->data[i];
        fwrite(bf->array, 1, bf->size, f);
//        for (int j = 0; j < bf->size; j++)
//            fputc( bf->array[j], f);
    }
    fclose(f);
}

BLOOMFILTER_TREE *load_bf_tree(char *filename) {
    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        fprintf(stderr, "File not found: %s\n", filename);
        exit(1);
    }

    char type = fgetc(f);
    char c;
    int size = 0, leaves = 0;
    fscanf(f, "%d:%d:", &size, &leaves);

    BLOOMFILTER_TREE *bft;
    if (type == 'v')
        bft = init_variable_bf_tree(leaves, size);
    else {
        bft = init_fixed_bf_tree(leaves, size);
        build_bf_tree(bft);
    }

    for (int i = 1; i < bft->size; i++) {
        BLOOMFILTER *bf = bft->data[i];
        fread(bf->array, 1, bf->size, f);
    }

    fclose(f);

    return bft;
}

void search_path_in_bf_tree(BLOOMFILTER_TREE *bft, char *filename) {
    DIR *dir;
    struct dirent *ent;
    const int max_path_length = 1024;

    char *cur_dir = (char *) malloc(max_path_length);
    getcwd(cur_dir, max_path_length);

    //in case of a dir
    if (is_dir(filename)) {
        dir = opendir(filename);
        chdir(filename);

        //run through all files of the dir
        while ((ent = readdir(dir)) != NULL) {
            //if we found a file, generate hash value and add it
            if (is_file(ent->d_name)) {
                search_path_in_bf_tree(bft, ent->d_name);
            } else if (is_dir(ent->d_name)) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    search_path_in_bf_tree(bft, ent->d_name);
            }
        }
        chdir(cur_dir);
        closedir(dir);
    }

        //in case we we have only a file
    else if (is_file(filename)) {
        FILE_CONTENTS *fc = read_file(filename);
        if ( fc != NULL ) {
            int *result = search(bft, fc);
            free(result); //TODO: don't use it for now, just prevent memory leak
            destroy_file_contents(fc);
#ifdef LOGGING
            printf("Searched for [%s]\n", filename);
#endif
        }
    }
}

void add_path_to_bf_tree(BLOOMFILTER_TREE *bft, char *filename) {
    DIR *dir;
    struct dirent *ent;
    const int max_path_length = 1024;

    char *cur_dir = (char *) malloc(max_path_length);
    getcwd(cur_dir, max_path_length);

    //in case of a dir
    if (is_dir(filename)) {

        dir = opendir(filename);
        chdir(filename);

        //run through all files of the dir
        while ((ent = readdir(dir)) != NULL) {
            //if we found a file, generate hash value and add it
            if (is_file(ent->d_name)) {
                add_path_to_bf_tree(bft, ent->d_name);
            } else if (is_dir(ent->d_name)) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    add_path_to_bf_tree(bft, ent->d_name);
            }
        }
        chdir(cur_dir);
        closedir(dir);
    } else if (is_file(filename)) {
        FILE_CONTENTS *fc = read_file(filename);
        if (fc != NULL) {
            int leaf = insert_into_bf_tree(bft, fc);
#ifdef LOGGING
            printf("Added [%s] to leaf [%d]\n", filename, leaf);
#endif
            destroy_file_contents(fc);
        }
#ifdef LOGGING
        else {
            printf("Skipping [%s] (zero bytes)\n", filename);
        }
#endif
    }
}