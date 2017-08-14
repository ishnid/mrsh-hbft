/* 
 * File:   main.c
 * Created on 28. April 2013, 19:15
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <limits.h>		/* PATH_MAX */
#include "../header/main.h"
#include "../header/helper.h"

#include <math.h>

// Global variable for the different modes
MODES *mode;

extern char *optarg;
extern int optind;


static void show_help (void) {
    printf ("\nmrsh-v2  by Frank Breitinger\n"
    		"Copyright (C) 2013 \n"
    		"\n"
    		"Usage: mrsh-net [-gh] [-s size] [-ia DB-FILE] DIR/FILE \n"
            "OPTIONS: -i: Reads DB-FILE and compares DIR/FILE against it. \n"
    		"         -a: Adds to an existing DB-FILE new files and prints it to std (save to a file './mrsh-net DIR > myDB' \n"
            "         -g: Generate the database and print it to std (save to a file './mrsh-net DIR > myDB' \n"
    		"             Note, only works with directory eg: ./ or myDir/ -- does not work with myDir/* \n"
    		"         -r: Reads directory recursive, default no \n"
    		"         -s: Calculates the size for a database of <size in MB> \n"
            "         -h: Print this help message \n\n");
}



static void initalizeDefaultModes(){
	mode = (MODES *)malloc(sizeof(MODES));
	mode->readDB = false;
	mode->helpmessage = false;
	mode->generateBF = false;
	mode->recursive = false;
	mode->calc_size = false;
	mode->addToDB = false;
	mode->combine = false;
}



// TODO: revert to main()
int main1(int argc, char **argv){

	int i;
	initalizeDefaultModes();
//	initialize_settings();		//Bloom filter settings

	char *listName = NULL;


	while ((i=getopt(argc,argv,"i:a:s:ghrc:")) != -1) {
	    switch(i) {
	    	case 'i':	mode->readDB = true; listName = optarg; break;
	    	case 'a':	mode->addToDB = true; listName = optarg; break;
	    	case 'c':	mode->combine = true; listName = optarg; break;
	    	case 'g':	mode->generateBF = true; break;
	    	case 'r':	mode->recursive = true; break;
	    	case 's':	mode->calc_size = true; listName = optarg; break;
	    	case 'h':	mode->helpmessage = true; break;
	    	default: 	mode->helpmessage = true;
	    				fprintf(stderr,"[*] Unknown option(s) \n");
	    				 break;
	    }
	  }


	if(mode->helpmessage) {
	    	show_help();
	    	exit(0);
	}

	if(mode->calc_size){
		calculate_size((int)atoi(listName));
		exit(1);
	}

	//read all arguments, create the bloomfilter, and print it to stdout
	if(mode->generateBF || optind==1) {
	  BLOOMFILTER *bf = init_empty_BF(BF_SIZE_IN_BYTES);
	  for (int j = optind; j < argc; j++)
		  addPathToBloomfilter(bf, argv[j]);
	  print_bf(bf);
	  destroy_bf(bf);
	  exit(1);
	}


	//read db from file and compare stuff against it
	if(mode->readDB){
		BLOOMFILTER *bf = init_empty_BF(BF_SIZE_IN_BYTES);
		readFileToBF(listName, bf);			//read in the "DB"
		comparePathToBloomfilter(bf, argv[optind]);
	}


	//read db from db and add files
	if(mode->addToDB){
		BLOOMFILTER *bf = init_empty_BF(BF_SIZE_IN_BYTES);
		readFileToBF(listName, bf);					//read in the "DB"
		for (int j = optind; j < argc; j++)
			addPathToBloomfilter(bf, argv[j]);		//add new files to existing "DB"
		print_bf(bf);
		destroy_bf(bf);
		exit(1);
	}
}



/*
 * adds a path to a fingerprints list. may be recursive depending on the parameters
 */
void addPathToBloomfilter(BLOOMFILTER *bf, char *filename){
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
		  			FILE *file = getFileHandle(ent->d_name);
		  			//printf("%s \n", ent->d_name);
		  			hashFileToBF(bf, file);
		  			fclose(file);
		  		}

		  		//when we found a dir and recursive mode is on, go deeper
		  		else if(is_dir(ent->d_name) && mode->recursive) {
		  			if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		  			    continue;
		  			addPathToBloomfilter(bf, ent->d_name);
		  		}
		  	}
		  	chdir(cur_dir);
		  	closedir (dir);
	}

	//in case we we have only a file
	else if(is_file(filename)) {
		FILE *file = getFileHandle(filename);
		hashFileToBF(bf, file);
		fclose(file);
	}

	return;
}




/*
 * adds a path to a fingerprints list. may be recursive depending on the parameters
 */
void comparePathToBloomfilter(BLOOMFILTER *bf, char *filename){
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
		  			FILE *file = getFileHandle(ent->d_name);
		  			evaluation(ent->d_name, hashFileAndCompare(bf, file));
		  			fclose(file);
		  		}

		  		//when we found a dir and recursive mode is on, go deeper
		  		else if(is_dir(ent->d_name) && mode->recursive) {
		  			if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		  			    continue;
		  			comparePathToBloomfilter(bf, ent->d_name);
		  		}
		  	}
		  	chdir(cur_dir);
		  	closedir (dir);
	}

	//in case we we have only a file
	else if(is_file(filename)) {
		FILE *file = getFileHandle(filename);
		evaluation(filename, hashFileAndCompare(bf, file));
		fclose(file);
	}

	return;
}


void evaluation(char *filename, int *results){
	if(results[2] > mode->min_run){
		printf("%s: %i of %i (longest run: %i) \n", filename, results[1], results[0], results[2]);
		//printf("%s: longest run %i \n", filename, results[2]);
	}
}


//Based on here:
//http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html
void calculate_size(int filesize){
	int logsize = (((int)log2(filesize))+1)*3; 	//exponent for file size in KiB
	int bs = (int)log2(mode->block_size);				//exponent for the estimated length of block

	int p_log_size = logsize-bs+5;
	int mb = powl(2,(p_log_size-20));

	printf("\n   Set BF-size in config.h to %i and 'make'. Needed memory: ~%i MB)\n "
			"      (approximate FP rate of 6.33e-05)\n", p_log_size, mb);
}

