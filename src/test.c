//
// Created by David Lillis on 08/09/2016.
//

#include "../header/bloomfilter.h"
#include "../header/bloomfiltertree.h"
#include "../header/helper.h"
#include "../header/hashing.h"
#include "../header/fingerprintList.h"
#include "../header/fingerprint.h"


#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>

void search_path(BLOOMFILTER_TREE *bft, char *filename){
    DIR *dir;
    struct dirent *ent;
    const int max_path_length = 1024;

    char *cur_dir = (char *)malloc(max_path_length);
    getcwd(cur_dir, max_path_length);

    //in case of a dir
    if (is_dir(filename)) {

        dir = opendir (filename);
        chdir(filename);

        //run through all files of the dir
        while ((ent = readdir (dir)) != NULL) {
            //if we found a file, generate hash value and add it
            if(is_file(ent->d_name)) {
                search_path(bft, ent->d_name);
            }
            else if(is_dir(ent->d_name)) {
                if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    search_path(bft, ent->d_name);
            }
        }
        chdir(cur_dir);
        closedir(dir);
    }

    else if(is_file(filename)) {
        FILE_CONTENTS *fc = read_file(filename);
        int *result = search(bft, fc);
        destroy_file_contents( fc );
    }
}

int main(int argc, char *argv[]) {

    if ( argc == 1 ) {
        printf("Usage: %s tree_dir search_dir block_size min_run leaf_num", argv[0]);
        exit(10);
    }

    mode = malloc(sizeof(MODES));

    char *tree_dirname = argv[1];

    // temporary parameters
    char *search_dirname = argv[2];

    // 2: block size (in bytes)
    mode->block_size = atoi(argv[3]);

    // 3: MIN_RUN
    mode->min_run = atoi(argv[4]);

    // 4: leaves
    int leaf_num = atoi(argv[5]);

    unsigned long mem_upper_limit = 1024ul * 1024ul * 1024ul * 10; // default to 10GiB

    // end temporary parameters

#ifdef FIXED_TREE
    unsigned long max_mem = mem_upper_limit / (leaf_num * 2 - 1);

    unsigned long root_bf_size = (unsigned long) pow(2, (unsigned long) log2(max_mem));

    BLOOMFILTER_TREE *tree = init_fixed_bf_tree(leaf_num, root_bf_size);

    mode->fixed = true;
#else

    unsigned long max_mem = mem_upper_limit / ( log2(leaf_num) + 1 );

    unsigned long root_bf_size = (unsigned long) pow(2, (unsigned long) log2(max_mem));
//    if ( root_bf_size > MAX_BF_SIZE_IN_BYTES )
//        root_bf_size = MAX_BF_SIZE_IN_BYTES;

    BLOOMFILTER_TREE *tree = init_variable_bf_tree(leaf_num, root_bf_size);
#endif

#ifdef LOGGING
    printf( "Upper memory limit: %lu\n", mem_upper_limit);
    printf("Config BLOCK_SIZE %d\nConfig MIN_RUN %d\nConfig LEAF_NUM %d\nConfig BF_SIZE %lu\nConfig FIXED %s\n", mode->block_size, mode->min_run, leaf_num, root_bf_size, mode->fixed ? "true" : "false");
#endif

    // start timing
    clock_t build_tic = clock();

    add_path_to_bf_tree(tree, tree_dirname);

#ifdef FIXED_TREE
    build_bf_tree(tree);
#endif
    clock_t build_toc = clock();

#ifdef LOGGING
    for ( int i = 1; i < tree->size; i++ ) {
        printf("Fullness %d %f\n", i, fullness(tree->data[i]));
        printf("Files %d (%lu bytes)\n", tree->data[i]->number_of_files, tree->data[i]->bytes);
    }
#endif

    clock_t search_tic = clock();
    search_path_in_bf_tree(tree, search_dirname);
    clock_t search_toc = clock();

#ifdef LOGGING
    printf("Build Time: %f seconds\nSearch Time: %f seconds\n", (double) (build_toc - build_tic) / CLOCKS_PER_SEC,
           (double) (search_toc - search_tic) / CLOCKS_PER_SEC);
#endif

//    for ( int i = 0; i < leaf_num; i++ )
//       printf( "Fill: %d %f\n", i, fullness(get_leaf_bf(tree, i)));


    /*
    FINGERPRINT_LIST *fpl = init_empty_fingerprintList();

    char *filename = "/Users/daithi/code/approximate_matching/t5/000004.doc";
    char *filename2 = "/Users/daithi/code/approximate_matching/t5/000005.doc";
    char *filename3 = "/Users/daithi/code/approximate_matching/t5/003569.jpg";

    FILE_CONTENTS *fc = read_file(filename);
    FILE_HASH *fh = hash_file(fc);

    add_new_fingerprint(fpl, init_fingerprint_for_file(fh));

    FILE_CONTENTS *fc2 = read_file(filename2);
    FILE_HASH *fh2 = hash_file(fc2);



    add_new_fingerprint(fpl, init_fingerprint_for_file(fh2));

//    all_against_all_comparsion(fpl);

//    print_fingerprint(fp);
//    print_fingerprint(fp2);

//    printf("%d\n", fingerprint_compare(fp, fp2));




    int leaf_num = 1;
////
    BLOOMFILTER_TREE *tree = init_fixed_bf_tree(leaf_num, BF_SIZE_IN_BYTES);
////    BLOOMFILTER_TREE *tree = init_variable_bf_tree(leaf_num, BF_SIZE_IN_BYTES);
//
//    char* *dirname = "/Users/daithi/code/approximate_matching/t5/";
//
//    add_path_to_bf_tree(tree, dirname);
////    search_path_in_bf_tree(tree, dirname);
//
   add_path_to_bf_tree(tree, filename3);
//
////    char *out_name = "/Users/daithi/test/test_bf.db";
////    save_bf_tree(out_name, tree);
//
////    BLOOMFILTER_TREE *tree = load_bf_tree( out_name );
//
   int *result = search(tree, read_file(filename));

    printf("searching\n");

    for( int i = 0; i < leaf_num; i++ ) {
        printf("%d: %d %d\n", i, result[i], count_ones(get_leaf_bf(tree, i)));
    }*/

    destroy_bf_tree(tree);
}
